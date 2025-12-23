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

    // 尝试购买种子：
    // - 校验种子类型与玩家金币/精力是否足够。
    // - 成功时扣除金币与精力，并向背包添加 1 个种子。
    // 返回 true 表示购买成功。
    bool buySeed(Game::ItemType seedType);

    // 获取种子价格：
    // - 当前为简单规则：回生作物种子更贵，其它为基础价。
    int getSeedPrice(Game::ItemType seedType) const;

    // 尝试购买一般物品（矿物/农产品/动物产物等）：
    // - 排除鱼与熟食（示例规则，避免与专门商店冲突）。
    // - 成功时扣除金币，并向背包添加 1 个物品。
    bool buyItem(Game::ItemType type);
    // 获取一般物品价格（当前委托给 Game::itemPrice）。
    int getItemPrice(Game::ItemType type) const;

    // 出售物品：从背包移除数量并按单价增加金币；qty 为出售数量。
    bool sellItem(Game::ItemType type, int qty);

private:
    std::shared_ptr<Game::Inventory> _inventory; // 交易目标背包（物品增删的唯一来源）
};

}
