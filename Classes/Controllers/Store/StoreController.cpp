#include "StoreController.h"
#include "Game/WorldState.h"
#include "Game/Crops/crop/CropBase.h"

namespace Controllers {

// 构造：持有共享背包指针，用于买入/卖出时直接修改玩家物品
StoreController::StoreController(std::shared_ptr<Game::Inventory> inventory)
    : _inventory(inventory) {}

// 购买种子：
// - 仅允许购买“种子类”物品（复用 Game::isSeed 判断）。
// - 成功时扣除金币与精力，并向背包添加 1 个对应物品。
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

// 获取种子价格：当前将“回生作物种子”设为更高单价（示例规则）。
int StoreController::getSeedPrice(Game::ItemType seedType) const {
    int base = 25;
    Game::CropType cropType = Game::cropTypeFromSeed(seedType);
    if (Game::CropDefs::isRegrow(cropType)) {
        return 50;
    }
    return base;
}

// 购买一般物品（矿物/农产品/动物产物等）：使用 Game::itemPrice 作为单价
bool StoreController::buyItem(Game::ItemType type) {
    if (Game::isFish(type)) {
        return false;
    }
    if (Game::isCookedFood(type)) {
        return false;
    }
    int price = getItemPrice(type);
    if (price <= 0) {
        return false;
    }
    auto& ws = Game::globalState();
    if (ws.gold >= price) {
        if (_inventory && _inventory->addItems(type, 1) == 0) {
            ws.gold -= price;
            return true;
        }
    }
    return false;
}

// 获取一般物品价格：
// - 默认委托给 Game::itemPrice。
// - 对鱼类按折扣价处理（示例规则）。
int StoreController::getItemPrice(Game::ItemType type) const {
    if (Game::isCookedFood(type)) {
        return 0;
    }
    int base = Game::itemPrice(type);
    if (Game::isFish(type)) {
        return (base * 6) / 10;
    }
    return base;
}

// 出售物品：
// - 禁止出售种子与熟食（示例规则）。
// - 先从背包扣除 qty，成功后再按单价增加金币，避免“扣钱成功但物品未移除”的不一致。
bool StoreController::sellItem(Game::ItemType type, int qty) {
    if (qty <= 0) return false;
    if (Game::isSeed(type)) return false;
    if (Game::isCookedFood(type)) return false;
    auto& ws = Game::globalState();
    int buyPrice = getItemPrice(type);
    if (buyPrice <= 0) return false;
    int unit = buyPrice;
    if (!Game::isFish(type)) {
        unit = (buyPrice * 7) / 10;
    }
    if (!_inventory) return false;
    bool removed = _inventory->removeItems(type, qty);
    if (!removed) return false;
    ws.gold += static_cast<long long>(unit) * qty;
    return true;
}

}
