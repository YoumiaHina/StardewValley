#pragma once

#include <unordered_map>
#include "Game/Item.h"

namespace Game {

// 简单背包：统计物品数量
class Bag {
public:
    void add(ItemType type, int qty = 1);
    int count(ItemType type) const;

    // 为 UI 提供全部条目快照（按枚举顺序）
    std::vector<std::pair<ItemType,int>> all() const;

private:
    std::unordered_map<ItemType, int, ItemTypeHash> _counts;
};

} // namespace Game