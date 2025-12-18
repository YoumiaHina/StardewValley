#include "Controllers/Systems/FurnaceController.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/PlaceableItemBase.h"
#include "Controllers/Interact/PlacementInteractor.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/Map/RoomMapController.h"
#include "Controllers/Map/TownMapController.h"
#include "Controllers/Map/BeachMapController.h"
#include "Controllers/UI/UIController.h"

using namespace cocos2d;

namespace Controllers {

namespace {

std::vector<Game::Furnace>* getFurnacesForMap(Controllers::IMapController* map) {
    if (!map) return nullptr;
    auto& ws = Game::globalState();
    if (map->isFarm()) {
        return &ws.farmFurnaces;
    }
    if (auto* room = dynamic_cast<Controllers::RoomMapController*>(map)) {
        (void)room;
        return &ws.houseFurnaces;
    }
    if (dynamic_cast<Controllers::TownMapController*>(map)) {
        return &ws.townFurnaces;
    }
    if (dynamic_cast<Controllers::BeachMapController*>(map)) {
        return &ws.beachFurnaces;
    }
    return nullptr;
}

}

FurnaceController::FurnaceController()
: PlaceableItemSystemBase()
, _map(nullptr)
, _ui(nullptr)
, _inventory(nullptr)
, _runtime(nullptr) {
}

void FurnaceController::attachTo(cocos2d::Node* parentNode, int zOrder) {
    PlaceableItemSystemBase::attachTo(parentNode, zOrder);
}

void FurnaceController::bindContext(Controllers::IMapController* map,
                                    Controllers::UIController* ui,
                                    std::shared_ptr<Game::Inventory> inventory) {
    if (map) {
        _map = map;
        if (!_runtime) {
            _runtime = getFurnacesForMap(_map);
        }
    }
    if (ui) {
        _ui = ui;
    }
    if (inventory) {
        _inventory = std::move(inventory);
    }
}

void FurnaceController::syncLoad() {
    if (!_runtime && _map) {
        _runtime = getFurnacesForMap(_map);
    }
    if (!_runtime || !_map) return;
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    for (auto& f : *_runtime) {
        if (f.dropOffset.isZero()) {
            f.dropOffset = Vec2(0, s * 0.5f);
        }
    }
    refreshVisuals();
}

void FurnaceController::update(float dt) {
    if (!_runtime || !_map) return;
    bool changedState = false;
    for (auto& f : *_runtime) {
        if (f.remainingSeconds > 0.0f) {
            float before = f.remainingSeconds;
            f.remainingSeconds -= dt;
            if (f.remainingSeconds <= 0.0f && before > 0.0f) {
                f.remainingSeconds = 0.0f;
                changedState = true;
                auto recipe = Game::furnaceRecipeFor(f.oreType);
                if (!Game::isValidFurnaceRecipe(recipe)) {
                    continue;
                }
                Game::ItemType ingot = recipe.output;
                Vec2 dropPos = f.pos + f.dropOffset;
                int c = 0;
                int r = 0;
                _map->worldToTileIndex(dropPos, c, r);
                _map->spawnDropAt(c, r, static_cast<int>(ingot), 1);
                _map->refreshDropsVisuals();
                if (_ui) {
                    Vec2 uiPos = _map->getPlayerPosition(dropPos);
                    _ui->popTextAt(uiPos, "Smelted", Color3B::YELLOW);
                }
            }
        }
    }
    if (changedState) {
        refreshVisuals();
    }
}

void FurnaceController::refreshVisuals() {
    if (!_drawNode || !_runtime) return;
    _drawNode->clear();
    _drawNode->removeAllChildren();
    for (const auto& f : *_runtime) {
        auto r = f.placeRect();
        Vec2 center(r.getMidX(), r.getMidY());
        const char* tex = f.remainingSeconds > 0.0f ? "PlaceableItem/Furnace_On.png" : "PlaceableItem/Furnace.png";
        auto spr = Sprite::create(tex);
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
            _drawNode->drawSolidPoly(v, 4, Color4F(0.5f, 0.5f, 0.5f, 0.9f));
        }
    }
}

const std::vector<Game::Furnace>& FurnaceController::furnaces() const {
    static const std::vector<Game::Furnace> empty;
    return _runtime ? *_runtime : empty;
}

std::vector<Game::Furnace>& FurnaceController::furnaces() {
    static std::vector<Game::Furnace> empty;
    return _runtime ? *_runtime : empty;
}

bool FurnaceController::interactAt(const cocos2d::Vec2& playerWorldPos,
                                   const cocos2d::Vec2& lastDir) {
    if (!_map || !_ui) return false;
    return InteractWithItem(_map, _ui, _inventory, playerWorldPos, lastDir);
}

bool FurnaceController::isNearFurnace(const cocos2d::Vec2& worldPos) const {
    if (!_runtime) return false;
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float margin = s * 0.4f;
    return Game::PlaceableItemBase::isNearAny<Game::Furnace>(
        worldPos,
        *_runtime,
        [](const Game::Furnace& f) { return f.placeRect(); },
        margin);
}

bool FurnaceController::collides(const cocos2d::Vec2& worldPos) const {
    if (!_runtime) return false;
    for (const auto& f : *_runtime) {
        if (f.collisionRect().containsPoint(worldPos)) {
            return true;
        }
    }
    return false;
}

bool FurnaceController::shouldPlace(const Game::Inventory& inventory) const {
    if (inventory.selectedKind() != Game::SlotKind::Item) return false;
    const auto& slot = inventory.selectedSlot();
    return slot.itemType == Game::ItemType::Furnace && slot.itemQty > 0;
}

bool FurnaceController::tryPlace(Controllers::IMapController* map,
                                 Controllers::UIController* ui,
                                 const std::shared_ptr<Game::Inventory>& inventory,
                                 const cocos2d::Vec2& playerWorldPos,
                                 const cocos2d::Vec2& lastDir) {
    if (!map || !ui || !inventory) return false;
    if (!_runtime) {
        _runtime = getFurnacesForMap(map);
    }
    if (!_runtime) return false;
    Vec2 center;
    bool okCenter = false;
    if (map->isFarm()) {
        okCenter = Controllers::PlaceablePlacementBase::selectFarmCenter(map, playerWorldPos, lastDir, center);
    } else {
        auto* room = dynamic_cast<Controllers::RoomMapController*>(map);
        if (room) {
            okCenter = Controllers::PlaceablePlacementBase::selectRoomCenter(room, playerWorldPos, center);
        } else {
            okCenter = Controllers::PlaceablePlacementBase::selectOutdoorCenter(map, playerWorldPos, center);
        }
    }
    if (!okCenter) return false;

    auto& furnaces = *_runtime;
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float margin = s * 0.4f;
    auto blocked = [map](const Rect& r) {
        auto* room = dynamic_cast<Controllers::RoomMapController*>(map);
        if (room) {
            return r.intersectsRect(room->doorRect()) || r.intersectsRect(room->bedRect());
        }
        return false;
    };
    bool okPlace = Game::PlaceableItemBase::canPlaceAt<Game::Furnace>(
        center,
        furnaces,
        [](const Game::Furnace& f) { return f.placeRect(); },
        blocked,
        margin);
    if (!okPlace) return false;

    if (furnaces.size() >= static_cast<std::size_t>(Game::Furnace().maxPerArea())) {
        Vec2 uiPos = map->getPlayerPosition(center);
        ui->popTextAt(uiPos, "Too many furnaces", Color3B::RED);
        return false;
    }

    Game::Furnace f;
    f.pos = center;
    f.dropOffset = Vec2(0, s * 0.5f);
    f.oreType = Game::ItemType::CopperGrain;
    f.remainingSeconds = 0.0f;
    furnaces.push_back(f);

    bool removed = inventory->removeItems(Game::ItemType::Furnace, 1);
    if (removed) {
        ui->refreshHotbar();
    }
    Vec2 uiPos = map->getPlayerPosition(center);
    ui->popTextAt(uiPos, "Placed Furnace", Color3B::YELLOW);
    refreshVisuals();
    return true;
}

int FurnaceController::findNearestFurnace(const cocos2d::Vec2& playerWorldPos, float maxDist) const {
    if (!_runtime) return -1;
    float maxDistSq = maxDist * maxDist;
    float best = maxDistSq;
    int bestIdx = -1;
    for (int i = 0; i < static_cast<int>(_runtime->size()); ++i) {
        const auto& f = (*_runtime)[static_cast<std::size_t>(i)];
        float d2 = playerWorldPos.distanceSquared(f.pos);
        if (d2 <= best) {
            best = d2;
            bestIdx = i;
        }
    }
    return bestIdx;
}

bool FurnaceController::tryInteractExisting(Controllers::IMapController* map,
                                            Controllers::UIController* ui,
                                            const std::shared_ptr<Game::Inventory>& inventory,
                                            const cocos2d::Vec2& playerWorldPos,
                                            const cocos2d::Vec2& lastDir) {
    (void)lastDir;
    if (!map || !ui || !inventory) return false;
    if (!_runtime) return false;
    int idx = findNearestFurnace(playerWorldPos, static_cast<float>(GameConfig::TILE_SIZE));
    if (idx < 0) return false;
    auto& f = (*_runtime)[static_cast<std::size_t>(idx)];
    if (f.remainingSeconds > 0.0f) {
        return false;
    }
    if (inventory->selectedKind() != Game::SlotKind::Item) {
        return false;
    }
    const auto& slot = inventory->selectedSlot();
    Game::ItemType ore = slot.itemType;
    auto recipe = Game::furnaceRecipeFor(ore);
    if (!Game::isValidFurnaceRecipe(recipe)) {
        return false;
    }
    int haveOre = inventory->countItems(recipe.ore);
    int haveFuel = inventory->countItems(recipe.fuel);
    if (haveOre < recipe.oreCount || haveFuel < recipe.fuelCount) {
        return false;
    }
    bool removedOre = inventory->removeItems(recipe.ore, recipe.oreCount);
    bool removedFuel = inventory->removeItems(recipe.fuel, recipe.fuelCount);
    if (!removedOre || !removedFuel) {
        return false;
    }
    f.oreType = recipe.ore;
    f.remainingSeconds = recipe.seconds;
    if (ui) {
        ui->refreshHotbar();
        Vec2 uiPos = map->getPlayerPosition(playerWorldPos);
        ui->popTextAt(uiPos, "Smelting...", Color3B::YELLOW);
    }
    refreshVisuals();
    return true;
}

}
