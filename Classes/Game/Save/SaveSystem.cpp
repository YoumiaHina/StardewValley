#include "Game/Save/SaveSystem.h"
#include "Game/WorldState.h"
#include "Game/Inventory.h"
#include "Game/Tool/ToolFactory.h"
#include "Game/Save/SaveDetail.h"
#include "cocos2d.h"
#include <fstream>
#include <sstream>
#include <limits>
#include <unordered_set>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <windows.h>
#endif

using namespace cocos2d;

namespace Game {

// g_customSaveRoot：可选的“自定义存档根目录”，由 setSaveRootDirectory 设置。
// g_currentSavePath：最近一次读/写存档实际使用的完整路径，供 UI 显示或调试查看。
static std::string g_customSaveRoot;
static std::string g_currentSavePath;

// 设置自定义存档根目录，不做路径拼接与合法性检查；
// 真正使用时会在 makeAbsoluteSaveRoot 中结合 FileUtils 做处理。
void setSaveRootDirectory(const std::string& rootDir) {
    g_customSaveRoot = rootDir;
}

// 记录当前存档的完整路径，通常在保存或加载成功时调用。
void setCurrentSavePath(const std::string& fullPath) {
    g_currentSavePath = fullPath;
}

// 返回当前记录的存档路径字符串。
std::string currentSavePath() {
    return g_currentSavePath;
}

// 辅助函数：规范化目录字符串，保证末尾一定带有斜杠 '/' 或 '\\'。
// 方便后续直接用“目录 + 文件名”拼接路径。
static std::string normalizeSaveDir(std::string dir) {
    if (!dir.empty()) {
        char last = dir[dir.size() - 1];
        if (last != '/' && last != '\\') {
            dir.push_back('/');
        }
    }
    return dir;
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
// Windows 专用：把宽字符 std::wstring 转成 UTF-8 编码的 std::string。
// 便于统一使用 UTF-8 字符串与 cocos2d::FileUtils 等接口交互。
static std::string utf8FromWide(const std::wstring& w) {
    if (w.empty()) return std::string();
    int size = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), static_cast<int>(w.size()), nullptr, 0, nullptr, nullptr);
    if (size <= 0) return std::string();
    std::string result(static_cast<std::size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), static_cast<int>(w.size()), &result[0], size, nullptr, nullptr);
    return result;
}

// Windows 专用：获得当前可执行文件所在的目录，作为相对路径的“基准目录”。
static std::string getExeDirectory() {
    WCHAR buffer[MAX_PATH + 1] = { 0 };
    DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (len == 0 || len == MAX_PATH) {
        return std::string();
    }
    WCHAR* lastSlash = wcsrchr(buffer, L'\\');
    if (lastSlash) {
        *(lastSlash + 1) = 0;
    }
    std::wstring wdir(buffer);
    return utf8FromWide(wdir);
}
#endif

// 把用户给出的 root 目录转成“绝对路径形式”的存档根目录：
// - 若 root 本身是绝对路径，直接规范化并返回；
// - 否则在 Windows 上优先拼接 exe 目录，其他平台使用 FileUtils::getWritablePath。
static std::string makeAbsoluteSaveRoot(const std::string& root) {
    auto* fu = FileUtils::getInstance();
    if (root.empty()) {
        return std::string();
    }
    if (fu->isAbsolutePath(root)) {
        return normalizeSaveDir(root);
    }
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    std::string base = getExeDirectory();
    if (base.empty()) {
        base = fu->getWritablePath();
    }
#else
    std::string base = fu->getWritablePath();
#endif
    base = normalizeSaveDir(base);
    return base + root;
}

// 过滤玩家输入的存档名称，生成一个“合法的文件名”：
// - 只允许数字/字母/空格/下划线/连字符/点号；
// - 其余字符统一替换成 '_'；
// - 去掉开头结尾的空格和点，避免在部分系统下出现隐藏文件等问题。
static std::string sanitizeFileName(const std::string& name) {
    std::string result;
    result.reserve(name.size());
    for (char c : name) {
        bool ok = false;
        if ((c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z')) {
            ok = true;
        }
        if (c == ' ' || c == '_' || c == '-' || c == '.') {
            ok = true;
        }
        if (ok) {
            result.push_back(c);
        } else {
            result.push_back('_');
        }
    }
    while (!result.empty() && (result.front() == ' ' || result.front() == '.')) {
        result.erase(result.begin());
    }
    while (!result.empty() && (result.back() == ' ' || result.back() == '.')) {
        result.pop_back();
    }
    if (result.empty()) {
        result = "NewSave";
    }
    return result;
}

// 判断字符串是否已经以 .txt（大小写不敏感）结尾。
static bool hasTxtExtension(const std::string& name) {
    if (name.size() < 4) return false;
    std::size_t n = name.size();
    return (name[n - 4] == '.' &&
            (name[n - 3] == 't' || name[n - 3] == 'T') &&
            (name[n - 2] == 'x' || name[n - 2] == 'X') &&
            (name[n - 1] == 't' || name[n - 1] == 'T'));
}

// 根据玩家输入的存档名称生成完整路径：
// - 前缀使用 saveDirectory()；
// - 文件名部分先 sanitizeFileName，再补全 .txt 后缀。
std::string makeSavePathWithName(const std::string& name) {
    std::string dir = normalizeSaveDir(saveDirectory());
    std::string base = sanitizeFileName(name);
    if (!hasTxtExtension(base)) {
        base += ".txt";
    }
    return dir + base;
}

// 计算并创建存档目录：
// - 若设置了 g_customSaveRoot，则转换成绝对路径后使用；
// - 否则在可写路径(writablePath)下创建 "save" 子目录。
// 返回值总是以斜杠结尾的目录路径。
std::string saveDirectory() {
    auto* fu = FileUtils::getInstance();
    std::string dir;
    if (!g_customSaveRoot.empty()) {
        dir = makeAbsoluteSaveRoot(g_customSaveRoot);
    } else {
        std::string base = fu->getWritablePath();
        base = normalizeSaveDir(base);
        dir = base + "save";
    }
    fu->createDirectory(dir);
    return normalizeSaveDir(dir);
}

// 根据存档槽位编号生成路径，比如 slot=1 -> ".../save1.txt"。
// 槽位在 [1, 50] 范围之外会被截断到合法范围。
std::string savePathForSlot(int slot) {
    if (slot < 1) slot = 1;
    if (slot > 50) slot = 50;
    std::string dir = normalizeSaveDir(saveDirectory());
    return dir + "save" + std::to_string(slot) + ".txt";
}

// 默认存档路径：约定为第 1 号槽位。
std::string defaultSavePath() {
    return savePathForSlot(1);
}

// 核心函数：把当前全局 WorldState 序列化到指定文件。
// 主要流程：
// 1. 处理 fullPath（为空则使用 defaultSavePath），并确保目录存在；
// 2. 打开 std::ofstream，写入存档版本号与基础数值字段；
// 3. 利用 SaveDetail.h 中的辅助函数写入 Inventory / Chest / Furnace / Crop 等列表。
bool saveToFile(const std::string& fullPath) {
    std::string path = fullPath;
    if (path.empty()) {
        path = defaultSavePath();
    } else {
        auto pos = path.find_last_of("/\\");
        if (pos != std::string::npos) {
            std::string dir = path.substr(0, pos);
            if (!dir.empty()) {
                FileUtils::getInstance()->createDirectory(dir);
            }
        }
    }
    g_currentSavePath = path;
    std::ofstream out(path, std::ios::trunc);
    if (!out) return false;
    auto& ws = globalState();
    out << "SDV_SAVE 10" << '\n';
    out << ws.seasonIndex << ' ' << ws.dayOfSeason << ' '
        << ws.timeHour << ' ' << ws.timeMinute << ' '
        << ws.timeAccum << ' '
        << ws.energy << ' ' << ws.maxEnergy << ' '
        << ws.water << ' ' << ws.maxWater << ' '
        << ws.gold << ' '
        << ws.hp << ' ' << ws.maxHp << ' '
        << ws.selectedIndex << ' '
        << (ws.grantedSwordAtEntrance ? 1 : 0) << ' '
        << (ws.fishingActive ? 1 : 0) << ' '
        << ws.lastScene << ' '
        << ws.lastPlayerX << ' '
        << ws.lastPlayerY << ' '
        << ws.lastMineFloor << ' '
        << ws.lastSaveSlot << ' '
        << (ws.isRaining ? 1 : 0) << ' '
        << ws.weatherSeasonIndex << ' '
        << ws.weatherDayOfSeason << ' '
        << ws.playerShirt << ' '
        << ws.playerPants << ' '
        << ws.playerHair << ' '
        << ws.playerHairR << ' '
        << ws.playerHairG << ' '
        << ws.playerHairB << '\n';
    out << ws.farmCols << ' ' << ws.farmRows << '\n';
    std::size_t tilesCount = ws.farmTiles.size();
    out << tilesCount << '\n';
    for (std::size_t i = 0; i < tilesCount; ++i) {
        out << toInt(ws.farmTiles[i]);
        if (i + 1 < tilesCount) {
            out << ' ';
        }
    }
    out << '\n';
    out << ws.abyssElevatorFloors.size() << '\n';
    for (int floor : ws.abyssElevatorFloors) {
        out << floor << '\n';
    }
    writeInventory(out, ws.inventory);
    writeChest(out, ws.globalChest);
    writeDrops(out, ws.farmDrops);
    writeFurnaces(out, ws.farmFurnaces);
    writeFurnaces(out, ws.houseFurnaces);
    writeFurnaces(out, ws.townFurnaces);
    writeFurnaces(out, ws.beachFurnaces);
    writeChests(out, ws.farmChests);
    writeCrops(out, ws.farmCrops);
    writeTreePositions(out, ws.farmTrees);
    writeRockPositions(out, ws.farmRocks);
    writeWeedPositions(out, ws.farmWeeds);
    writeAnimals(out, ws.farmAnimals);
    writeSkillTrees(out, ws.skillTrees);
    writeChests(out, ws.houseChests);
    writeChests(out, ws.townChests);
    writeChests(out, ws.beachChests);
    writeNpcData(out, ws);
    return true;
}

// 从文件中加载存档到全局 WorldState：
// 1. 处理 fullPath（为空则使用默认路径），尝试打开输入文件；
// 2. 读取并校验“魔数 + 版本号”，不符合期望则立即返回 false；
// 3. 依次读取基础字段、农田格子、矿井电梯层数、背包、箱子、熔炉等；
// 4. 根据 version 做向后兼容处理，例如老版本没有某些字段时给默认值。
bool loadFromFile(const std::string& fullPath) {
    std::string path = fullPath;
    if (path.empty()) {
        path = defaultSavePath();
    }
    std::ifstream in(path);
    if (!in) return false;
    g_currentSavePath = path;
    std::string magic;
    int version = 0;
    in >> magic >> version;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (!in || magic != "SDV_SAVE" || version < 7 || version > 10) {
        return false;
    }
    auto& ws = globalState();
    ws = WorldState();
    int granted = 0;
    int fishing = 0;
    int raining = 0;
    in >> ws.seasonIndex >> ws.dayOfSeason
       >> ws.timeHour >> ws.timeMinute
       >> ws.timeAccum
       >> ws.energy >> ws.maxEnergy
       >> ws.water >> ws.maxWater
       >> ws.gold
       >> ws.hp >> ws.maxHp
       >> ws.selectedIndex
       >> granted
       >> fishing;
    if (version >= 2) {
        in >> ws.lastScene
           >> ws.lastPlayerX
           >> ws.lastPlayerY
           >> ws.lastMineFloor
           >> ws.lastSaveSlot;
    }
    if (version >= 5) {
        in >> raining;
    }
    if (version >= 6) {
        in >> ws.weatherSeasonIndex >> ws.weatherDayOfSeason;
    }
    if (version >= 7) {
        in >> ws.playerShirt
           >> ws.playerPants
           >> ws.playerHair
           >> ws.playerHairR
           >> ws.playerHairG
           >> ws.playerHairB;
    }
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    ws.grantedSwordAtEntrance = (granted != 0);
    ws.fishingActive = (fishing != 0);
    ws.isRaining = (raining != 0);
    if (version < 6) {
        ws.weatherSeasonIndex = -1;
        ws.weatherDayOfSeason = -1;
    }
    in >> ws.farmCols >> ws.farmRows;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::size_t tilesCount = 0;
    in >> tilesCount;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    ws.farmTiles.clear();
    std::vector<long long> legacyRockTiles;
    std::vector<long long> legacyTreeTiles;
    if (tilesCount > 0) {
        ws.farmTiles.resize(tilesCount);
        for (std::size_t i = 0; i < tilesCount; ++i) {
            int v = 0;
            in >> v;
            ws.farmTiles[i] = tileFromInt(v);
            if (ws.farmCols > 0) {
                int c = static_cast<int>(i % static_cast<std::size_t>(ws.farmCols));
                int r = static_cast<int>(i / static_cast<std::size_t>(ws.farmCols));
                long long key = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
                if (v == 3) legacyRockTiles.push_back(key);
                if (v == 4) legacyTreeTiles.push_back(key);
            }
        }
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::size_t elevatorCount = 0;
    in >> elevatorCount;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    ws.abyssElevatorFloors.clear();
    for (std::size_t i = 0; i < elevatorCount; ++i) {
        int floor = 0;
        in >> floor;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        ws.abyssElevatorFloors.insert(floor);
    }
    ws.inventory = readInventory(in);
    ws.globalChest = readChest(in);
    readDrops(in, ws.farmDrops);
    if (version >= 3) {
        readFurnaces(in, ws.farmFurnaces);
        if (version >= 4) {
            readFurnaces(in, ws.houseFurnaces);
            readFurnaces(in, ws.townFurnaces);
            readFurnaces(in, ws.beachFurnaces);
        } else {
            ws.houseFurnaces.clear();
            ws.townFurnaces.clear();
            ws.beachFurnaces.clear();
        }
    } else {
        ws.farmFurnaces.clear();
        ws.houseFurnaces.clear();
        ws.townFurnaces.clear();
        ws.beachFurnaces.clear();
    }
    readChests(in, ws.farmChests);
    readCrops(in, ws.farmCrops);
    readTreePositions(in, ws.farmTrees);
    readRockPositions(in, ws.farmRocks);
    if (ws.farmRocks.empty() && !legacyRockTiles.empty()) {
        std::unordered_set<long long> seen;
        ws.farmRocks.reserve(legacyRockTiles.size());
        for (long long key : legacyRockTiles) {
            if (seen.count(key)) continue;
            seen.insert(key);
            int r = static_cast<int>(key >> 32);
            int c = static_cast<int>(key & 0xFFFFFFFF);
            ws.farmRocks.push_back(Game::RockPos{c, r, Game::RockKind::Rock1});
        }
    }
    if (ws.farmTrees.empty() && !legacyTreeTiles.empty()) {
        std::unordered_set<long long> seen;
        ws.farmTrees.reserve(legacyTreeTiles.size());
        for (long long key : legacyTreeTiles) {
            if (seen.count(key)) continue;
            seen.insert(key);
            int r = static_cast<int>(key >> 32);
            int c = static_cast<int>(key & 0xFFFFFFFF);
            ws.farmTrees.push_back(Game::TreePos{c, r, Game::TreeKind::Tree1});
        }
    }
    if (version >= 8) {
        readWeedPositions(in, ws.farmWeeds);
    } else {
        ws.farmWeeds.clear();
    }
    readAnimals(in, ws.farmAnimals);
    if (version >= 10) {
        readSkillTrees(in, ws.skillTrees);
    } else if (version >= 9) {
        readSkillTreesV4(in, ws.skillTrees);
    } else {
        for (auto& st : ws.skillTrees) {
            st.totalXp = 0;
            st.unspentPoints = 0;
            st.unlockedNodeIds.clear();
        }
    }
    readChests(in, ws.houseChests);
    readChests(in, ws.townChests);
    readChests(in, ws.beachChests);
    readNpcData(in, ws);
    return static_cast<bool>(in);
}

} // namespace Game
