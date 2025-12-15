#include "Game/Chest.h"
#include "Game/WorldState.h"
#include "Game/Inventory.h"
#include "Controllers/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Map/RoomMapController.h"
#include <functional>

using namespace cocos2d;

namespace Game {

Rect chestRect(const Chest& chest) {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    Vec2 center = chest.pos;
    float w = s;
    float h = s * 2.0f;
    float minX = center.x - w * 0.5f;
    float minY = center.y - h * 0.5f;
    return Rect(minX, minY, w, h);
}

bool isNearAnyChest(const Vec2& playerWorldPos, const std::vector<Chest>& chests) {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float margin = s * 0.4f;
    for (const auto& ch : chests) {
        auto r = chestRect(ch);
        Rect expanded(r.getMinX() - margin,
                      r.getMinY() - margin,
                      r.size.width + margin * 2.0f,
                      r.size.height + margin * 2.0f);
        if (expanded.containsPoint(playerWorldPos)) return true;
    }
    return false;
}

static int nearestChestIndex(const Vec2& playerWorldPos, const std::vector<Chest>& chests) {
    int idx = -1;
    float best = 1e9f;
    for (int i = 0; i < static_cast<int>(chests.size()); ++i) {
        float d = playerWorldPos.distance(chests[i].pos);
        if (d < best) {
            best = d;
            idx = i;
        }
    }
    return idx;
}

namespace {

bool placeChestCommon(const Vec2& center,
                      Controllers::UIController* ui,
                      const std::shared_ptr<Game::Inventory>& inventory,
                      std::vector<Game::Chest>& chs,
                      const std::function<bool(const Rect&)>& blocked,
                      bool isFarm,
                      Controllers::IMapController* map,
                      Controllers::RoomMapController* room) {
    if (!ui || !inventory) return false;
    if (inventory->selectedKind() != Game::SlotKind::Item) return false;
    const auto& slot = inventory->selectedSlot();
    if (slot.itemType != Game::ItemType::Chest || slot.itemQty <= 0) return false;

    Game::Chest tmpChest;
    tmpChest.pos = center;
    Rect candidateRect = chestRect(tmpChest);
    if (blocked && blocked(candidateRect)) return false;

    if (isNearAnyChest(center, chs)) return false;

    Game::Chest chest;
    chest.pos = center;
    chs.push_back(chest);

    auto& ws = Game::globalState();
    if (isFarm) {
        ws.farmChests = chs;
        if (map) {
            map->refreshMapVisuals();
        }
    } else {
        ws.houseChests = chs;
        if (room) {
            room->refreshChestsVisuals();
        }
    }

    inventory->removeItems(Game::ItemType::Chest, 1);
    ui->refreshHotbar();
    ui->popTextAt(center, "Placed Chest", Color3B::YELLOW);
    return true;
}

} // anonymous namespace

bool openChestNearPlayer(Controllers::IMapController* map,
                         Controllers::UIController* ui,
                         const Vec2& playerWorldPos,
                         const Vec2& lastDir) {
    if (!map || !ui) return false;
    if (!map->isFarm()) return false;
    auto& chs = map->chests();
    if (chs.empty()) return false;
    auto tgt = map->targetTile(playerWorldPos, lastDir);
    int tc = tgt.first;
    int tr = tgt.second;
    Vec2 center = map->tileToWorld(tc, tr);
    int idx = -1;
    for (int i = 0; i < static_cast<int>(chs.size()); ++i) {
        auto r = chestRect(chs[i]);
        if (r.containsPoint(center)) {
            idx = i;
            break;
        }
    }
    if (idx < 0) return false;
    ui->buildChestPanel();
    ui->refreshChestPanel(&chs[idx]);
    ui->toggleChestPanel(true);
    return true;
}

bool placeChestOnFarm(Controllers::IMapController* map,
                      Controllers::UIController* ui,
                      std::shared_ptr<Game::Inventory> inventory,
                      const Vec2& playerPos,
                      const Vec2& lastDir) {
    if (!map || !ui || !inventory) return false;
    if (!map->isFarm()) return false;
    auto tgt = map->targetTile(playerPos, lastDir);
    int tc = tgt.first;
    int tr = tgt.second;
    auto& chs = map->chests();
    auto blocked = [](const Rect&) { return false; };
    Vec2 center = map->tileToWorld(tc, tr);
    return placeChestCommon(center, ui, inventory, chs, blocked, true, map, nullptr);
}

bool placeChestInRoom(Controllers::RoomMapController* room,
                      Controllers::UIController* ui,
                      std::shared_ptr<Game::Inventory> inventory,
                      const Vec2& playerPos) {
    (void)room;
    (void)ui;
    (void)inventory;
    (void)playerPos;
    return false;
}

void transferChestCell(Game::Chest& chest,
                       int flatIndex,
                       Game::Inventory& inventory) {
    if (flatIndex < 0) return;
    if (chest.slots.empty()) {
        chest.slots.resize(static_cast<std::size_t>(Chest::CAPACITY));
    }
    if (flatIndex >= static_cast<int>(chest.slots.size())) return;
    Slot& cs = chest.slots[static_cast<std::size_t>(flatIndex)];

    if (cs.kind == SlotKind::Item && cs.itemQty > 0) {
        ItemType type = cs.itemType;
        int qty = cs.itemQty;
        int remaining = inventory.addItems(type, qty);
        int moved = qty - remaining;
        if (moved <= 0) return;
        cs.itemQty -= moved;
        if (cs.itemQty <= 0) {
            cs.kind = SlotKind::Empty;
            cs.itemQty = 0;
        }
    } else {
        if (inventory.selectedKind() != SlotKind::Item) return;
        auto const& sel = inventory.selectedSlot();
        if (sel.itemQty <= 0) return;
        ItemType type = sel.itemType;
        int qty = sel.itemQty;

        if (cs.kind == SlotKind::Empty) {
            cs.kind = SlotKind::Item;
            cs.itemType = type;
            cs.itemQty = 0;
        } else if (!(cs.kind == SlotKind::Item && cs.itemType == type)) {
            return;
        }

        bool ok = inventory.consumeSelectedItem(qty);
        if (!ok) return;
        cs.itemQty += qty;
    }

    auto& ws = Game::globalState();
    for (auto& ch : ws.farmChests) {
        if (ch.pos.equals(chest.pos)) {
            ch = chest;
        }
    }
    for (auto& ch : ws.houseChests) {
        if (ch.pos.equals(chest.pos)) {
            ch = chest;
        }
    }
}

} // namespace Game

