#include "StoreController.h"
#include "Game/WorldState.h"
#include "Game/Crop.h"

namespace Controllers {

StoreController::StoreController(std::shared_ptr<Game::Inventory> inventory)
    : _inventory(inventory) {}

bool StoreController::buySeed(Game::ItemType seedType) {
    // 简单校验是否为种子（虽然逻辑上允许买任何东西，但需求是种子商店）
    if (!Game::isSeed(seedType)) return false;

    int price = getSeedPrice(seedType);
    auto& ws = Game::globalState();

    if (ws.gold >= price && ws.energy >= 2) {
        if (_inventory && _inventory->addItems(seedType, 1) == 0) {
            ws.gold -= price;
            ws.energy -= 2; if (ws.energy < 0) ws.energy = 0;
            return true;
        }
    }
    return false;
}

int StoreController::getSeedPrice(Game::ItemType seedType) const {
    // 需求：种子商店，25Gold一个
    return 25;
}

bool StoreController::buyItem(Game::ItemType type) {
    // 非种子物品：使用物品自身价格；不消耗能量
    int price = getItemPrice(type);
    auto& ws = Game::globalState();
    if (ws.gold >= price) {
        if (_inventory && _inventory->addItems(type, 1) == 0) {
            ws.gold -= price;
            return true;
        }
    }
    return false;
}

int StoreController::getItemPrice(Game::ItemType type) const {
    return Game::itemPrice(type);
}

}
