#include "Controllers/Systems/ChestController.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/PlaceableItemBase.h"
#include "Game/Tool/ToolFactory.h"
#include "Controllers/Interact/PlacementInteractor.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/Map/RoomMapController.h"
#include "Controllers/Map/TownMapController.h"
#include "Controllers/Map/BeachMapController.h"
#include "Controllers/UI/UIController.h"
#include <functional>

using namespace cocos2d;

namespace Controllers {

void ChestController::attachTo(Node* parentNode, int zOrder) {
    PlaceableItemSystemBase::attachTo(parentNode, zOrder);
}

void ChestController::syncLoad() {
    auto& ws = Game::globalState();
    if (_isFarm) {
        _chests = ws.farmChests;
    } else {
        _chests = ws.houseChests;
    }
}

const std::vector<Game::Chest>& ChestController::chests() const {
    return _chests;
}

std::vector<Game::Chest>& ChestController::chests() {
    return _chests;
}

bool ChestController::isNearChest(const Vec2& worldPos) const {
    return Game::isNearAnyChest(worldPos, _chests);
}

bool ChestController::collides(const Vec2& worldPos) const {
    return Game::PlaceableItemBase::collidesAny<Game::Chest>(
        worldPos,
        _chests,
        [](const Game::Chest& c) { return Game::chestCollisionRect(c); });
}

void ChestController::refreshVisuals() {
    if (!_drawNode) return;
    _drawNode->clear();
    _drawNode->removeAllChildren();
    for (const auto& ch : _chests) {
        auto r = Game::chestRect(ch);
        Vec2 center(r.getMidX(), r.getMidY());
        auto spr = Sprite::create("Chest.png");
        if (spr && spr->getTexture()) {
            auto cs = spr->getContentSize();
            if (cs.width > 0 && cs.height > 0) {
                float sx = r.size.width / cs.width;
                float sy = r.size.height / cs.height;
                float scale = std::min(sx, sy);
                spr->setScale(scale);
            }
            spr->setPosition(center);
            _drawNode->addChild(spr);
        } else {
            Vec2 a(r.getMinX(), r.getMinY());
            Vec2 b(r.getMaxX(), r.getMinY());
            Vec2 c(r.getMaxX(), r.getMaxY());
            Vec2 d(r.getMinX(), r.getMaxY());
            Vec2 v[4] = { a, b, c, d };
            _drawNode->drawSolidPoly(v, 4, Color4F(0.6f,0.4f,0.2f,0.9f));
        }
    }
}

bool ChestController::shouldPlace(const Game::Inventory& inventory) const {
    if (inventory.selectedKind() != Game::SlotKind::Item) return false;
    const auto& slot = inventory.selectedSlot();
    return slot.itemType == Game::ItemType::Chest && slot.itemQty > 0;
}

bool ChestController::tryPlace(Controllers::IMapController* map,
                               Controllers::UIController* ui,
                               const std::shared_ptr<Game::Inventory>& inventory,
                               const Vec2& playerWorldPos,
                               const Vec2& lastDir) {
    if (!map || !ui || !inventory) return false;
    if (map->isFarm()) {
        return placeChestOnFarm(map, ui, inventory, playerWorldPos, lastDir);
    }
    auto* room = dynamic_cast<Controllers::RoomMapController*>(map);
    if (room) {
        return placeChestInRoom(room, ui, inventory, playerWorldPos);
    }
    return placeChestOnOutdoorMap(map, ui, inventory, playerWorldPos);
}

bool ChestController::tryInteractExisting(Controllers::IMapController* map,
                                          Controllers::UIController* ui,
                                          const std::shared_ptr<Game::Inventory>& inventory,
                                          const Vec2& playerWorldPos,
                                          const Vec2& lastDir) {
    (void)inventory;
    return openChestNearPlayer(map, ui, playerWorldPos, lastDir);
}

namespace {

using ChestSyncFunc = std::function<void(const std::vector<Game::Chest>&)>;

int nearestChestIndex(const Vec2& playerWorldPos, const std::vector<Game::Chest>& chests) {
    int idx = -1;
    float best = 1e9f;
    for (int i = 0; i < static_cast<int>(chests.size()); ++i) {
        float d = playerWorldPos.distance(chests[static_cast<std::size_t>(i)].pos);
        if (d < best) {
            best = d;
            idx = i;
        }
    }
    return idx;
}

bool placeChestCommon(const Vec2& center,
                      Controllers::UIController* ui,
                      const std::shared_ptr<Game::Inventory>& inventory,
                      std::vector<Game::Chest>& chs,
                      const std::function<bool(const Rect&)>& blocked,
                      Controllers::IMapController* map,
                      Controllers::RoomMapController* room,
                      const ChestSyncFunc& syncWorld) {
    if (!ui || !inventory) return false;
    if (inventory->selectedKind() != Game::SlotKind::Item) return false;
    const auto& slot = inventory->selectedSlot();
    if (slot.itemType != Game::ItemType::Chest || slot.itemQty <= 0) return false;

    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float margin = s * 0.4f;
    bool okPlace = Game::PlaceableItemBase::canPlaceAt<Game::Chest>(
        center,
        chs,
        [](const Game::Chest& c) { return Game::chestRect(c); },
        blocked,
        margin);
    if (!okPlace) return false;

    if (chs.size() >= static_cast<std::size_t>(Game::Chest::MAX_PER_AREA)) {
        Vec2 uiPos = center;
        if (map) uiPos = map->getPlayerPosition(center);
        else if (room) uiPos = room->getPlayerPosition(center);
        ui->popTextAt(uiPos, "Too many chests", Color3B::RED);
        return false;
    }

    Game::Chest chest;
    chest.pos = center;
    chs.push_back(chest);

    if (syncWorld) {
        syncWorld(chs);
    }

    inventory->removeItems(Game::ItemType::Chest, 1);
    ui->refreshHotbar();

    Vec2 uiPos = center;
    if (map) uiPos = map->getPlayerPosition(center);
    else if (room) uiPos = room->getPlayerPosition(center);
    ui->popTextAt(uiPos, "Placed Chest", Color3B::YELLOW);
    return true;
}

} // anonymous namespace

bool openChestNearPlayer(Controllers::IMapController* map,
                         Controllers::UIController* ui,
                         const Vec2& playerWorldPos,
                         const Vec2& lastDir) {
    if (!map || !ui) return false;
    auto& chs = map->chests();
    if (chs.empty()) return false;
    int idx = -1;
    if (map->isFarm()) {
        auto tgt = map->targetTile(playerWorldPos, lastDir);
        int tc = tgt.first;
        int tr = tgt.second;
        if (!map->inBounds(tc, tr)) return false;
        Vec2 center = map->tileToWorld(tc, tr);
        for (int i = 0; i < static_cast<int>(chs.size()); ++i) {
            auto r = Game::chestRect(chs[static_cast<std::size_t>(i)]);
            if (r.containsPoint(center)) {
                idx = i;
                break;
            }
        }
    } else {
        if (!Game::isNearAnyChest(playerWorldPos, chs)) return false;
        idx = nearestChestIndex(playerWorldPos, chs);
    }
    if (idx < 0) return false;
    Game::Chest* chest = &chs[static_cast<std::size_t>(idx)];
    ui->buildChestPanel();
    ui->refreshChestPanel(chest);
    ui->toggleChestPanel(true);
    return true;
}

bool openGlobalChest(Controllers::UIController* ui) {
    if (!ui) return false;
    auto& ws = Game::globalState();
    ui->buildChestPanel();
    ui->refreshChestPanel(&ws.globalChest);
    ui->toggleChestPanel(true);
    return true;
}

bool placeChestOnFarm(Controllers::IMapController* map,
                      Controllers::UIController* ui,
                      std::shared_ptr<Game::Inventory> inventory,
                      const Vec2& playerPos,
                      const Vec2& lastDir) {
    if (!map || !ui || !inventory) return false;
    Vec2 center;
    bool okCenter = Controllers::PlaceablePlacementBase::selectFarmCenter(map, playerPos, lastDir, center);
    if (!okCenter) return false;
    auto& chs = map->chests();
    auto blocked = [](const Rect&) { return false; };
    auto& ws = Game::globalState();
    ChestSyncFunc syncWorld = [&ws, map](const std::vector<Game::Chest>& out) {
        ws.farmChests = out;
        if (map) {
            map->refreshMapVisuals();
        }
    };
    return placeChestCommon(center, ui, inventory, chs, blocked, map, nullptr, syncWorld);
}

bool placeChestInRoom(Controllers::RoomMapController* room,
                      Controllers::UIController* ui,
                      std::shared_ptr<Game::Inventory> inventory,
                      const Vec2& playerPos) {
    if (!room || !ui || !inventory) return false;
    Vec2 center;
    bool okCenter = Controllers::PlaceablePlacementBase::selectRoomCenter(room, playerPos, center);
    if (!okCenter) return false;
    auto& chs = room->chests();
    auto blocked = [room](const Rect& r) {
        return r.intersectsRect(room->doorRect()) || r.intersectsRect(room->bedRect());
    };
    auto& ws = Game::globalState();
    ChestSyncFunc syncWorld = [&ws, room](const std::vector<Game::Chest>& out) {
        ws.houseChests = out;
        if (room) {
            room->refreshChestsVisuals();
        }
    };
    return placeChestCommon(center, ui, inventory, chs, blocked, nullptr, room, syncWorld);
}

bool placeChestOnOutdoorMap(Controllers::IMapController* map,
                            Controllers::UIController* ui,
                            std::shared_ptr<Game::Inventory> inventory,
                            const Vec2& playerPos) {
    if (!map || !ui || !inventory) return false;
    auto* town = dynamic_cast<Controllers::TownMapController*>(map);
    auto* beach = dynamic_cast<Controllers::BeachMapController*>(map);
    if (!town && !beach) {
        return false;
    }
    Vec2 center;
    bool okCenter = Controllers::PlaceablePlacementBase::selectOutdoorCenter(map, playerPos, center);
    if (!okCenter) return false;
    auto& ws = Game::globalState();
    std::vector<Game::Chest>* container = nullptr;
    ChestSyncFunc syncWorld;
    if (town) {
        container = &ws.townChests;
        syncWorld = [&ws](const std::vector<Game::Chest>& out) {
            ws.townChests = out;
        };
    } else {
        container = &ws.beachChests;
        syncWorld = [&ws](const std::vector<Game::Chest>& out) {
            ws.beachChests = out;
        };
    }
    auto& chs = *container;
    auto blocked = [](const Rect&) { return false; };
    return placeChestCommon(center, ui, inventory, chs, blocked, map, nullptr, syncWorld);
}

void transferChestCell(Game::Chest& chest,
                       int flatIndex,
                       Game::Inventory& inventory) {
    if (flatIndex < 0) return;
    if (chest.slots.empty()) {
        chest.slots.resize(static_cast<std::size_t>(Game::Chest::CAPACITY));
    }
    if (flatIndex >= static_cast<int>(chest.slots.size())) return;
    Game::Slot& cs = chest.slots[static_cast<std::size_t>(flatIndex)];
    int invIndex = inventory.selectedIndex();
    if (invIndex < 0 || invIndex >= static_cast<int>(inventory.size())) return;
    Game::SlotKind invKind = inventory.selectedKind();
    bool invIsItem = inventory.isItem(static_cast<std::size_t>(invIndex));
    bool invIsTool = inventory.isTool(static_cast<std::size_t>(invIndex));
    bool invEmpty = inventory.isEmpty(static_cast<std::size_t>(invIndex));
    Game::ItemStack invStack = inventory.itemAt(static_cast<std::size_t>(invIndex));
    bool chestHasItem = (cs.kind == Game::SlotKind::Item && cs.itemQty > 0);
    bool chestHasTool = (cs.kind == Game::SlotKind::Tool && cs.tool != nullptr);

    if (invIsItem && invStack.quantity > 0) {
        if (chestHasTool) return;
        if (chestHasItem && cs.itemType != invStack.type) return;
        if (!chestHasItem) {
            cs.kind = Game::SlotKind::Item;
            cs.itemType = invStack.type;
            cs.itemQty = 0;
            chestHasItem = true;
        }
        if (cs.itemQty >= Game::ItemStack::MAX_STACK) return;
        bool ok = inventory.consumeSelectedItem(1);
        if (!ok) return;
        cs.itemQty += 1;
    } else if (invIsTool) {
        if (chestHasItem || chestHasTool) return;
        const Game::ToolBase* tConst = inventory.toolAt(static_cast<std::size_t>(invIndex));
        if (!tConst) return;
        Game::ToolKind tk = tConst->kind();
        cs.kind = Game::SlotKind::Tool;
        cs.tool = Game::makeTool(tk);
        cs.itemQty = 0;
        bool cleared = inventory.clearSlot(static_cast<std::size_t>(invIndex));
        if (!cleared) {
            cs.tool.reset();
            cs.kind = Game::SlotKind::Empty;
            cs.itemQty = 0;
            return;
        }
    } else {
        if (chestHasItem) {
            Game::ItemType type = cs.itemType;
            bool canReceive = false;
            if (invEmpty) {
                canReceive = true;
            } else if (invIsItem && invStack.type == type && invStack.quantity < Game::ItemStack::MAX_STACK) {
                canReceive = true;
            }
            if (!canReceive) return;
            bool okAdd = inventory.addOneItemToSlot(static_cast<std::size_t>(invIndex), type);
            if (!okAdd) return;
            if (cs.itemQty <= 0) return;
            cs.itemQty -= 1;
            if (cs.itemQty <= 0) {
                cs.kind = Game::SlotKind::Empty;
                cs.itemQty = 0;
            }
        } else if (chestHasTool) {
            if (!invEmpty) return;
            if (!cs.tool) return;
            Game::ToolKind tk = cs.tool->kind();
            inventory.setTool(static_cast<std::size_t>(invIndex), Game::makeTool(tk));
            cs.tool.reset();
            cs.kind = Game::SlotKind::Empty;
            cs.itemQty = 0;
        } else {
            return;
        }
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
    for (auto& ch : ws.townChests) {
        if (ch.pos.equals(chest.pos)) {
            ch = chest;
        }
    }
    for (auto& ch : ws.beachChests) {
        if (ch.pos.equals(chest.pos)) {
            ch = chest;
        }
    }
    if (&chest == &ws.globalChest) {
        ws.globalChest = chest;
    }
}

}
