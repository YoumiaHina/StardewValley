#pragma once

#include <memory>
#include <vector>
#include <limits>
#include <string>
#include "Game/WorldState.h"
#include "Game/Inventory.h"
#include "Game/Tool/ToolFactory.h"
#include "Game/Chest.h"
#include "Game/Drop.h"
#include "Game/Crops/crop/CropBase.h"
#include "Game/Tree.h"
#include "Game/Rock.h"
#include "Game/Animal.h"
#include "Game/Furnace.h"
#include "cocos2d.h"

namespace {

int toInt(Game::TileType t) {
    return static_cast<int>(t);
}

Game::TileType tileFromInt(int v) {
    switch (v) {
        case 0: return Game::TileType::Soil;
        case 1: return Game::TileType::Tilled;
        case 2: return Game::TileType::Watered;
        case 3: return Game::TileType::Rock;
        case 4: return Game::TileType::Tree;
        case 5: return Game::TileType::NotSoil;
        default: return Game::TileType::Soil;
    }
}

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
        int itemType = 0;
        int qty = 0;
        if (inv->isTool(i)) {
            kind = 1;
            if (auto t = inv->toolAt(i)) {
                tk = static_cast<int>(t->kind());
            }
        } else if (inv->isItem(i)) {
            kind = 2;
            auto stack = inv->itemAt(i);
            itemType = static_cast<int>(stack.type);
            qty = stack.quantity;
        }
        out << kind << ' ' << tk << ' ' << itemType << ' ' << qty << '\n';
    }
}

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
        int itemType = 0;
        int qty = 0;
        in >> kind >> tk >> itemType >> qty;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        if (kind == 1) {
            auto tool = Game::makeTool(static_cast<Game::ToolKind>(tk));
            inv->setTool(static_cast<std::size_t>(i), tool);
        } else if (kind == 2) {
            int remaining = inv->addItems(static_cast<Game::ItemType>(itemType), qty);
            (void)remaining;
        }
    }
    return inv;
}

void writeChest(std::ostream& out, const Game::Chest& chest) {
    out << chest.pos.x << ' ' << chest.pos.y << ' ' << chest.slots.size() << '\n';
    for (const auto& s : chest.slots) {
        int kind = static_cast<int>(s.kind);
        int tk = 0;
        if (s.tool) {
            tk = static_cast<int>(s.tool->kind());
        }
        int itemType = static_cast<int>(s.itemType);
        int qty = s.itemQty;
        out << kind << ' ' << tk << ' ' << itemType << ' ' << qty << '\n';
    }
}

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
        int itemType = 0;
        int qty = 0;
        in >> kind >> tk >> itemType >> qty;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        auto& s = chest.slots[static_cast<std::size_t>(i)];
        s.kind = static_cast<Game::SlotKind>(kind);
        s.itemType = static_cast<Game::ItemType>(itemType);
        s.itemQty = qty;
        s.tool.reset();
        if (s.kind == Game::SlotKind::Tool) {
            s.tool = Game::makeTool(static_cast<Game::ToolKind>(tk));
        }
    }
    return chest;
}

void writeDrops(std::ostream& out, const std::vector<Game::Drop>& drops) {
    out << drops.size() << '\n';
    for (const auto& d : drops) {
        out << static_cast<int>(d.type) << ' ' << d.pos.x << ' ' << d.pos.y << ' ' << d.qty << '\n';
    }
}

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

void writeFurnaces(std::ostream& out, const std::vector<Game::Furnace>& furnaces) {
    out << furnaces.size() << '\n';
    for (const auto& f : furnaces) {
        out << f.pos.x << ' ' << f.pos.y << ' '
            << static_cast<int>(f.oreType) << ' '
            << f.remainingSeconds << '\n';
    }
}

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

void writeChests(std::ostream& out, const std::vector<Game::Chest>& chests) {
    out << chests.size() << '\n';
    for (const auto& ch : chests) {
        writeChest(out, ch);
    }
}

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

void writeCrops(std::ostream& out, const std::vector<Game::Crop>& crops) {
    out << crops.size() << '\n';
    for (const auto& cp : crops) {
        out << cp.c << ' ' << cp.r << ' ' << static_cast<int>(cp.type) << ' '
            << cp.stage << ' ' << cp.progress << ' ' << cp.maxStage << ' '
            << (cp.wateredToday ? 1 : 0) << '\n';
    }
}

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

void writeTreePositions(std::ostream& out, const std::vector<Game::TreePos>& trees) {
    out << trees.size() << '\n';
    for (const auto& t : trees) {
        out << t.c << ' ' << t.r << '\n';
    }
}

void readTreePositions(std::istream& in, std::vector<Game::TreePos>& trees) {
    std::size_t count = 0;
    in >> count;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    trees.clear();
    trees.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        int c = 0;
        int r = 0;
        in >> c >> r;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        Game::TreePos tp;
        tp.c = c;
        tp.r = r;
        trees.push_back(tp);
    }
}

void writeRockPositions(std::ostream& out, const std::vector<Game::RockPos>& rocks) {
    out << rocks.size() << '\n';
    for (const auto& r : rocks) {
        out << r.c << ' ' << r.r << '\n';
    }
}

void readRockPositions(std::istream& in, std::vector<Game::RockPos>& rocks) {
    std::size_t count = 0;
    in >> count;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    rocks.clear();
    rocks.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        int c = 0;
        int r = 0;
        in >> c >> r;
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!in) break;
        Game::RockPos rp;
        rp.c = c;
        rp.r = r;
        rocks.push_back(rp);
    }
}

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
