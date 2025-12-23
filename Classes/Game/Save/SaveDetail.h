#pragma once

#include <memory>
#include <array>
#include <vector>
#include <limits>
#include <string>
#include "Game/WorldState.h"
#include "Game/Inventory.h"
#include "Game/Tool/ToolFactory.h"
#include "Game/PlaceableItem/Chest.h"
#include "Game/Drop.h"
#include "Game/Crops/crop/CropBase.h"
#include "Game/EnvironmentObstacle/Tree.h"
#include "Game/EnvironmentObstacle/Rock.h"
#include "Game/Animals/Animal.h"
#include "Game/PlaceableItem/Furnace.h"
#include "cocos2d.h"

// 本文件中都是“读写存档细节”的工具函数：
// - 负责把 WorldState/Chest/Inventory 等结构体序列化成纯文本（写入输出流）
// - 或者从文本中解析回结构体（从输入流读取）
// 这里放在头文件中，供 SaveSystem.cpp 直接包含使用。
// 注意：下方使用匿名命名空间（namespace { ... }），等价于 C 语言里的 static 函数，
// 这些函数只在包含它的 .cpp 文件内部可见，不会污染全局命名空间。
namespace {

// 把地图格子的枚举类型 Game::TileType 转成 int，方便写入文本存档。
// 存档只保存数字，加载时再反向转换回枚举。
int toInt(Game::TileType t) {
    switch (t) {
        case Game::TileType::Soil: return 1;
        case Game::TileType::Tilled: return 2;
        case Game::TileType::Watered: return 3;
        case Game::TileType::NotSoil: return 0;
        default: return 0;
    }
}

// 与上面的 toInt 相反：把文件中读到的整数还原成 Game::TileType 枚举值。
// 非法数值统一当成 NotSoil 处理，保证程序健壮性。
Game::TileType tileFromInt(int v) {
    switch (v) {
        case 1: return Game::TileType::Soil;
        case 2: return Game::TileType::Tilled;
        case 3: return Game::TileType::Watered;
        case 0: return Game::TileType::NotSoil;
        default: return Game::TileType::NotSoil;
    }
}

// 把玩家背包 Inventory 写入输出流 out。
// - 使用引用 std::ostream& 作为“输出管道”，就像 C 语言里的 FILE*；
// - 使用 std::shared_ptr<Game::Inventory> 共享拥有背包对象，可能有多个模块同时持有它。
// 写入格式：
//   hasInv size
//   kind tk tl itemType qty  (一行一个格子)
// 其中 kind 表示该格子是“空 / 工具 / 物品”三种情况。
void writeInventory(std::ostream& out, const std::shared_ptr<Game::Inventory>& inv) {
    if (!inv) {
        out << 0 << ' ' << 0 << '\n';
        return;
    }
    std::size_t sz = inv->size();
    out << 1 << ' ' << sz << '\n';
    for (std::size_t i = 0; i < sz; ++i) {
        int kind = 0;
        int tk = 0;
        int tl = 0;
        int itemType = 0;
        int qty = 0;
        if (inv->isTool(i)) {
            kind = 1;
            if (auto t = inv->toolAt(i)) {
                tk = static_cast<int>(t->kind());
                tl = t->level();
            }
        } else if (inv->isItem(i)) {
            kind = 2;
            auto stack = inv->itemAt(i);
            itemType = static_cast<int>(stack.type);
            qty = stack.quantity;
        }
        out << kind << ' ' << tk << ' ' << tl << ' ' << itemType << ' ' << qty << '\n';
    }
}

// 从输入流 in 中读回一个 Inventory 对象。
// - 使用 std::istream& 作为“输入管道”；
// - 返回值是 std::shared_ptr<Game::Inventory>，读取失败时返回 nullptr。
// 读取时按照 writeInventory 写入时约定的顺序依次解析。
std::shared_ptr<Game::Inventory> readInventory(std::istream& in) {
    int hasInv = 0;
    int sz = 0;
    in >> hasInv >> sz;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (!in || hasInv == 0 || sz <= 0) {
        return nullptr;
    }
    auto inv = std::make_shared<Game::Inventory>(static_cast<std::size_t>(sz));
    for (int i = 0; i < sz; ++i) {
        int kind = 0;
        int tk = 0;
        int tl = 0;
        int itemType = 0;
        int qty = 0;
        in >> kind >> tk >> tl >> itemType >> qty;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        if (kind == 1) {
            auto tool = Game::makeTool(static_cast<Game::ToolKind>(tk));
            if (tool) {
                tool->setLevel(tl);
            }
            inv->setTool(static_cast<std::size_t>(i), tool);
        } else if (kind == 2) {
            int remaining = inv->addItems(static_cast<Game::ItemType>(itemType), qty);
            (void)remaining;
        }
    }
    return inv;
}

// 把一个 Game::Chest（包含位置 + 槽位数组）写入存档。
// 这里直接访问 chest.pos 和 chest.slots：
// - pos 是 cocos2d::Vec2，用于记住箱子的世界坐标；
// - slots 是一个 std::vector<ChestSlot>，记录每个格子的工具/物品。
void writeChest(std::ostream& out, const Game::Chest& chest) {
    out << chest.pos.x << ' ' << chest.pos.y << ' ' << chest.slots.size() << '\n';
    for (const auto& s : chest.slots) {
        int kind = static_cast<int>(s.kind);
        int tk = 0;
        int tl = 0;
        if (s.tool) {
            tk = static_cast<int>(s.tool->kind());
            tl = s.tool->level();
        }
        int itemType = static_cast<int>(s.itemType);
        int qty = s.itemQty;
        out << kind << ' ' << tk << ' ' << tl << ' ' << itemType << ' ' << qty << '\n';
    }
}

// 从输入流中读回一个 Chest：
// - 先读位置坐标和格子数量；
// - 再按顺序恢复每个槽位的 kind / tool / itemType / itemQty 等字段。
// 返回值按值传递（按值返回一个 Chest 对象）。
Game::Chest readChest(std::istream& in) {
    Game::Chest chest;
    float x = 0.0f;
    float y = 0.0f;
    int slots = 0;
    in >> x >> y >> slots;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    chest.pos = cocos2d::Vec2(x, y);
    chest.slots.clear();
    if (slots < 0) slots = 0;
    chest.slots.resize(static_cast<std::size_t>(slots));
    for (int i = 0; i < slots; ++i) {
        int kind = 0;
        int tk = 0;
        int tl = 0;
        int itemType = 0;
        int qty = 0;
        in >> kind >> tk >> tl >> itemType >> qty;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        auto& s = chest.slots[static_cast<std::size_t>(i)];
        s.kind = static_cast<Game::SlotKind>(kind);
        s.itemType = static_cast<Game::ItemType>(itemType);
        s.itemQty = qty;
        s.tool.reset();
        if (s.kind == Game::SlotKind::Tool) {
            s.tool = Game::makeTool(static_cast<Game::ToolKind>(tk));
            if (s.tool) {
                s.tool->setLevel(tl);
            }
        }
    }
    return chest;
}

// 把地面掉落物列表写入存档：
// - 先写掉落数量；
// - 再逐个写出 type（物品类型枚举）、位置坐标、数量。
// 使用 std::vector<Game::Drop> 存储一组掉落记录。
void writeDrops(std::ostream& out, const std::vector<Game::Drop>& drops) {
    out << drops.size() << '\n';
    for (const auto& d : drops) {
        out << static_cast<int>(d.type) << ' ' << d.pos.x << ' ' << d.pos.y << ' ' << d.qty << '\n';
    }
}

// 从存档中读取掉落物列表：
// - 先读取总数，再循环读取每一条记录；
// - 通过 static_cast<Game::ItemType>(type) 把 int 转回枚举类型。
void readDrops(std::istream& in, std::vector<Game::Drop>& drops) {
    std::size_t count = 0;
    in >> count;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    drops.clear();
    drops.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        int type = 0;
        float x = 0.0f;
        float y = 0.0f;
        int qty = 0;
        in >> type >> x >> y >> qty;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        Game::Drop d;
        d.type = static_cast<Game::ItemType>(type);
        d.pos = cocos2d::Vec2(x, y);
        d.qty = qty;
        drops.push_back(d);
    }
}

// 把熔炉列表写入存档：
// - 每个熔炉记录位置 pos、正在熔的矿石类型 oreType、剩余时间 remainingSeconds。
void writeFurnaces(std::ostream& out, const std::vector<Game::Furnace>& furnaces) {
    out << furnaces.size() << '\n';
    for (const auto& f : furnaces) {
        out << f.pos.x << ' ' << f.pos.y << ' '
            << static_cast<int>(f.oreType) << ' '
            << f.remainingSeconds << '\n';
    }
}

// 从存档中读取熔炉列表：
// - 使用 std::vector::reserve 预分配容量，避免多次扩容；
// - 对于枚举字段 oreType 同样使用 static_cast 转回枚举值。
void readFurnaces(std::istream& in, std::vector<Game::Furnace>& furnaces) {
    std::size_t count = 0;
    in >> count;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    furnaces.clear();
    furnaces.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        float x = 0.0f;
        float y = 0.0f;
        int ore = 0;
        float remaining = 0.0f;
        in >> x >> y >> ore >> remaining;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        Game::Furnace f;
        f.pos = cocos2d::Vec2(x, y);
        f.oreType = static_cast<Game::ItemType>(ore);
        f.remainingSeconds = remaining;
        f.dropOffset = cocos2d::Vec2::ZERO;
        furnaces.push_back(f);
    }
}

// 批量写入多个箱子：
// - 先写箱子数量；
// - 再循环调用 writeChest(out, ch) 写每一个箱子的详细内容。
void writeChests(std::ostream& out, const std::vector<Game::Chest>& chests) {
    out << chests.size() << '\n';
    for (const auto& ch : chests) {
        writeChest(out, ch);
    }
}

// 批量读取多个箱子：
// - 先读数量，再多次调用 readChest(in) 还原每一个箱子；
// - 读失败时提前退出循环。
void readChests(std::istream& in, std::vector<Game::Chest>& chests) {
    std::size_t count = 0;
    in >> count;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    chests.clear();
    chests.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        Game::Chest ch = readChest(in);
        if (!in) break;
        chests.push_back(ch);
    }
}

// 写入农作物列表：
// - 每个 Game::Crop 保存坐标(c, r)、作物类型 type、生长阶段 stage、进度 progress 等。
void writeCrops(std::ostream& out, const std::vector<Game::Crop>& crops) {
    out << crops.size() << '\n';
    for (const auto& cp : crops) {
        out << cp.c << ' ' << cp.r << ' ' << static_cast<int>(cp.type) << ' '
            << cp.stage << ' ' << cp.progress << ' ' << cp.maxStage << ' '
            << (cp.wateredToday ? 1 : 0) << '\n';
    }
}

// 读取农作物列表：
// - 注意使用 static_cast<Game::CropType>(type) 把 int 转回枚举；
// - wateredToday 字段使用 0/1 存储，再转换成 bool。
void readCrops(std::istream& in, std::vector<Game::Crop>& crops) {
    std::size_t count = 0;
    in >> count;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    crops.clear();
    crops.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        int c = 0;
        int r = 0;
        int type = 0;
        int stage = 0;
        int progress = 0;
        int maxStage = 0;
        int watered = 0;
        in >> c >> r >> type >> stage >> progress >> maxStage >> watered;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        Game::Crop cp;
        cp.c = c;
        cp.r = r;
        cp.type = static_cast<Game::CropType>(type);
        cp.stage = stage;
        cp.progress = progress;
        cp.maxStage = maxStage;
        cp.wateredToday = (watered != 0);
        crops.push_back(cp);
    }
}

// 写入树木位置列表：
// - 这里只保存位置和树种枚举 kind，不涉及运行时的节点指针。
void writeTreePositions(std::ostream& out, const std::vector<Game::TreePos>& trees) {
    out << trees.size() << '\n';
    for (const auto& t : trees) {
        out << t.c << ' ' << t.r << ' ' << static_cast<int>(t.kind) << '\n';
    }
}

// 读取树木位置列表：
// - 为了兼容旧存档，若读到非法 kind，就强制改为默认的 Tree1。
void readTreePositions(std::istream& in, std::vector<Game::TreePos>& trees) {
    std::size_t count = 0;
    in >> count;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    trees.clear();
    trees.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        int c = 0;
        int r = 0;
        int kind = 0;
        in >> c >> r >> kind;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        Game::TreePos tp;
        tp.c = c;
        tp.r = r;
        if (kind == static_cast<int>(Game::TreeKind::Tree1) ||
            kind == static_cast<int>(Game::TreeKind::Tree2)) {
            tp.kind = static_cast<Game::TreeKind>(kind);
        } else {
            tp.kind = Game::TreeKind::Tree1;
        }
        trees.push_back(tp);
    }
}

// 写入石头位置列表：同样只保存格子坐标和 RockKind 枚举。
void writeRockPositions(std::ostream& out, const std::vector<Game::RockPos>& rocks) {
    out << rocks.size() << '\n';
    for (const auto& r : rocks) {
        out << r.c << ' ' << r.r << ' ' << static_cast<int>(r.kind) << '\n';
    }
}

// 读取石头位置列表，并对非法 kind 做兜底处理，默认成 Rock1。
void readRockPositions(std::istream& in, std::vector<Game::RockPos>& rocks) {
    std::size_t count = 0;
    in >> count;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    rocks.clear();
    rocks.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        int c = 0;
        int r = 0;
        int kind = 0;
        in >> c >> r >> kind;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        Game::RockPos rp;
        rp.c = c;
        rp.r = r;
        if (kind == static_cast<int>(Game::RockKind::Rock1) ||
            kind == static_cast<int>(Game::RockKind::Rock2)) {
            rp.kind = static_cast<Game::RockKind>(kind);
        } else {
            rp.kind = Game::RockKind::Rock1;
        }
        rocks.push_back(rp);
    }
}

// 写入杂草位置列表：结构简单，只有格子坐标(c, r)。
void writeWeedPositions(std::ostream& out, const std::vector<Game::WeedPos>& weeds) {
    out << weeds.size() << '\n';
    for (const auto& w : weeds) {
        out << w.c << ' ' << w.r << '\n';
    }
}

// 读取杂草位置列表：依次读入每个格子的行列号，压入 std::vector 中。
void readWeedPositions(std::istream& in, std::vector<Game::WeedPos>& weeds) {
    std::size_t count = 0;
    in >> count;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    weeds.clear();
    weeds.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        int c = 0;
        int r = 0;
        in >> c >> r;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        Game::WeedPos wp;
        wp.c = c;
        wp.r = r;
        weeds.push_back(wp);
    }
}

// 写入农场动物列表：
// - Game::Animal 包含类型(type)、当前位置/目标位置(pos/target)、移动速度、游走半径、
//   年龄天数、是否成年、今天是否喂食等字段。
void writeAnimals(std::ostream& out, const std::vector<Game::Animal>& animals) {
    out << animals.size() << '\n';
    for (const auto& a : animals) {
        out << static_cast<int>(a.type) << ' '
            << a.pos.x << ' ' << a.pos.y << ' '
            << a.target.x << ' ' << a.target.y << ' '
            << a.speed << ' ' << a.wanderRadius << ' '
            << a.ageDays << ' '
            << (a.isAdult ? 1 : 0) << ' '
            << (a.fedToday ? 1 : 0) << '\n';
    }
}

// 读取农场动物列表，按与 writeAnimals 相同的顺序依次解析字段。
void readAnimals(std::istream& in, std::vector<Game::Animal>& animals) {
    std::size_t count = 0;
    in >> count;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    animals.clear();
    animals.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        int type = 0;
        float px = 0.0f;
        float py = 0.0f;
        float tx = 0.0f;
        float ty = 0.0f;
        float speed = 0.0f;
        float radius = 0.0f;
        int ageDays = 0;
        int isAdult = 0;
        int fedToday = 0;
        in >> type >> px >> py >> tx >> ty >> speed >> radius >> ageDays >> isAdult >> fedToday;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        Game::Animal a;
        a.type = static_cast<Game::AnimalType>(type);
        a.pos = cocos2d::Vec2(px, py);
        a.target = cocos2d::Vec2(tx, ty);
        a.speed = speed;
        a.wanderRadius = radius;
        a.ageDays = ageDays;
        a.isAdult = (isAdult != 0);
        a.fedToday = (fedToday != 0);
        animals.push_back(a);
    }
}

void writeSkillTrees(std::ostream& out, const std::array<Game::WorldState::SkillTreeProgress, 6>& skillTrees) {
    for (const auto& st : skillTrees) {
        out << st.totalXp << ' ' << st.unspentPoints << ' ' << st.unlockedNodeIds.size() << '\n';
        for (std::size_t i = 0; i < st.unlockedNodeIds.size(); ++i) {
            out << st.unlockedNodeIds[i];
            if (i + 1 < st.unlockedNodeIds.size()) out << ' ';
        }
        out << '\n';
    }
}

void readSkillTrees(std::istream& in, std::array<Game::WorldState::SkillTreeProgress, 6>& skillTrees) {
    for (auto& st : skillTrees) {
        int totalXp = 0;
        int unspentPoints = 0;
        std::size_t nodeCount = 0;
        in >> totalXp >> unspentPoints >> nodeCount;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        st.totalXp = totalXp;
        st.unspentPoints = unspentPoints;
        st.unlockedNodeIds.clear();
        st.unlockedNodeIds.reserve(nodeCount);
        for (std::size_t i = 0; i < nodeCount; ++i) {
            int id = 0;
            in >> id;
            st.unlockedNodeIds.push_back(id);
        }
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void readSkillTreesV4(std::istream& in, std::array<Game::WorldState::SkillTreeProgress, 6>& skillTrees) {
    for (auto& st : skillTrees) {
        st.totalXp = 0;
        st.unspentPoints = 0;
        st.unlockedNodeIds.clear();
    }
    for (int i = 0; i < 4; ++i) {
        int totalXp = 0;
        int unspentPoints = 0;
        std::size_t nodeCount = 0;
        in >> totalXp >> unspentPoints >> nodeCount;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        auto& st = skillTrees[static_cast<std::size_t>(i)];
        st.totalXp = totalXp;
        st.unspentPoints = unspentPoints;
        st.unlockedNodeIds.clear();
        st.unlockedNodeIds.reserve(nodeCount);
        for (std::size_t n = 0; n < nodeCount; ++n) {
            int id = 0;
            in >> id;
            st.unlockedNodeIds.push_back(id);
        }
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void writeNpcData(std::ostream& out, const Game::WorldState& ws) {
    out << ws.npcFriendship.size() << '\n';
    for (const auto& kv : ws.npcFriendship) {
        out << kv.first << ' ' << kv.second << '\n';
    }
    out << ws.npcRomanceUnlocked.size() << '\n';
    for (const auto& kv : ws.npcRomanceUnlocked) {
        out << kv.first << ' ' << (kv.second ? 1 : 0) << '\n';
    }
    out << ws.npcLastGiftDay.size() << '\n';
    for (const auto& kv : ws.npcLastGiftDay) {
        out << kv.first << ' ' << kv.second << '\n';
    }
    out << ws.npcQuests.size() << '\n';
    for (const auto& kv : ws.npcQuests) {
        out << kv.first << ' ' << kv.second.size() << '\n';
        for (const auto& q : kv.second) {
            out << q.title << '\n';
            out << q.description << '\n';
        }
    }
}

void readNpcData(std::istream& in, Game::WorldState& ws) {
    std::size_t count = 0;
    in >> count;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    ws.npcFriendship.clear();
    for (std::size_t i = 0; i < count; ++i) {
        int id = 0;
        int value = 0;
        in >> id >> value;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        ws.npcFriendship[id] = value;
    }
    in >> count;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    ws.npcRomanceUnlocked.clear();
    for (std::size_t i = 0; i < count; ++i) {
        int id = 0;
        int value = 0;
        in >> id >> value;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        ws.npcRomanceUnlocked[id] = (value != 0);
    }
    in >> count;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    ws.npcLastGiftDay.clear();
    for (std::size_t i = 0; i < count; ++i) {
        int id = 0;
        int value = 0;
        in >> id >> value;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        ws.npcLastGiftDay[id] = value;
    }
    in >> count;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    ws.npcQuests.clear();
    for (std::size_t i = 0; i < count; ++i) {
        int id = 0;
        std::size_t questCount = 0;
        in >> id >> questCount;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        std::vector<Game::NpcQuest> quests;
        quests.reserve(questCount);
        for (std::size_t q = 0; q < questCount; ++q) {
            std::string title;
            std::string desc;
            std::getline(in, title);
            std::getline(in, desc);
            if (!in) break;
            Game::NpcQuest quest;
            quest.title = title;
            quest.description = desc;
            quests.push_back(quest);
        }
        ws.npcQuests[id] = quests;
    }
}

} 
