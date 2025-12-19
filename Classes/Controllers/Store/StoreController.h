#pragma once

#include "Game/Inventory.h"
#include "Game/Item.h"
#include <memory>

namespace Controllers {

// 商店控制器：
// - 负责买入/卖出时的金币、精力与背包物品变更。
// - 不负责 UI 绘制与交互；UI 通过调用本控制器接口完成交易。
// - 协作对象：Inventory（物品增删）、WorldState（金币/精力）。
class StoreController {
public:
    // 构造：持有背包引用用于交易时增删物品。
    StoreController(std::shared_ptr<Game::Inventory> inventory);

    // 尝试购买种子
    // 返回 true 表示购买成功（扣钱并添加物品）
    bool buySeed(Game::ItemType seedType);

    // 获取种子价格
    int getSeedPrice(Game::ItemType seedType) const;

    // 购买一般物品（矿物等），价格来自 Game::itemPrice(type)
    bool buyItem(Game::ItemType type);
    // 获取一般物品价格（当前委托给 Game::itemPrice）。
    int getItemPrice(Game::ItemType type) const;

    // 出售物品：从背包移除数量并按单价增加金币；qty 为出售数量。
    bool sellItem(Game::ItemType type, int qty);

private:
    std::shared_ptr<Game::Inventory> _inventory;
};

}
