#pragma once

#include "Game/Inventory.h"
#include "Game/Item.h"
#include <memory>

namespace Controllers {

class StoreController {
public:
    StoreController(std::shared_ptr<Game::Inventory> inventory);

    // 尝试购买种子
    // 返回 true 表示购买成功（扣钱并添加物品）
    bool buySeed(Game::ItemType seedType);

    // 获取种子价格
    int getSeedPrice(Game::ItemType seedType) const;

private:
    std::shared_ptr<Game::Inventory> _inventory;
};

}
