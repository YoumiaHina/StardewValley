#include "Game/Save/SaveSystem.h"
#include "Game/WorldState.h"
#include "Game/Inventory.h"
#include "Game/Tool/ToolFactory.h"
#include "Game/Save/SaveDetail.h"
#include "cocos2d.h"
#include <fstream>
#include <sstream>
#include <limits>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <windows.h>
#endif

using namespace cocos2d;

namespace Game {

static std::string g_customSaveRoot;
static std::string g_currentSavePath;

void setSaveRootDirectory(const std::string& rootDir) {
    g_customSaveRoot = rootDir;
}

void setCurrentSavePath(const std::string& fullPath) {
    g_currentSavePath = fullPath;
}

std::string currentSavePath() {
    return g_currentSavePath;
}

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
static std::string utf8FromWide(const std::wstring& w) {
    if (w.empty()) return std::string();
    int size = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), static_cast<int>(w.size()), nullptr, 0, nullptr, nullptr);
    if (size <= 0) return std::string();
    std::string result(static_cast<std::size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), static_cast<int>(w.size()), &result[0], size, nullptr, nullptr);
    return result;
}

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

static bool hasTxtExtension(const std::string& name) {
    if (name.size() < 4) return false;
    std::size_t n = name.size();
    return (name[n - 4] == '.' &&
            (name[n - 3] == 't' || name[n - 3] == 'T') &&
            (name[n - 2] == 'x' || name[n - 2] == 'X') &&
            (name[n - 1] == 't' || name[n - 1] == 'T'));
}

std::string makeSavePathWithName(const std::string& name) {
    std::string dir = normalizeSaveDir(saveDirectory());
    std::string base = sanitizeFileName(name);
    if (!hasTxtExtension(base)) {
        base += ".txt";
    }
    return dir + base;
}

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

std::string savePathForSlot(int slot) {
    if (slot < 1) slot = 1;
    if (slot > 50) slot = 50;
    std::string dir = normalizeSaveDir(saveDirectory());
    return dir + "save" + std::to_string(slot) + ".txt";
}

std::string defaultSavePath() {
    return savePathForSlot(1);
}

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
    out << "SDV_SAVE 2" << '\n';
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
        << ws.lastSaveSlot << '\n';
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
    writeChests(out, ws.farmChests);
    writeCrops(out, ws.farmCrops);
    writeTreePositions(out, ws.farmTrees);
    writeRockPositions(out, ws.farmRocks);
    writeAnimals(out, ws.farmAnimals);
    writeChests(out, ws.houseChests);
    writeChests(out, ws.townChests);
    writeChests(out, ws.beachChests);
    writeNpcData(out, ws);
    return true;
}

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
    if (!in || magic != "SDV_SAVE" || (version != 1 && version != 2)) {
        return false;
    }
    auto& ws = globalState();
    ws = WorldState();
    int granted = 0;
    int fishing = 0;
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
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    ws.grantedSwordAtEntrance = (granted != 0);
    ws.fishingActive = (fishing != 0);
    in >> ws.farmCols >> ws.farmRows;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::size_t tilesCount = 0;
    in >> tilesCount;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    ws.farmTiles.clear();
    if (tilesCount > 0) {
        ws.farmTiles.resize(tilesCount);
        for (std::size_t i = 0; i < tilesCount; ++i) {
            int v = 0;
            in >> v;
            ws.farmTiles[i] = tileFromInt(v);
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
    readChests(in, ws.farmChests);
    readCrops(in, ws.farmCrops);
    readTreePositions(in, ws.farmTrees);
    readRockPositions(in, ws.farmRocks);
    readAnimals(in, ws.farmAnimals);
    readChests(in, ws.houseChests);
    readChests(in, ws.townChests);
    readChests(in, ws.beachChests);
    readNpcData(in, ws);
    return static_cast<bool>(in);
}

} // namespace Game
