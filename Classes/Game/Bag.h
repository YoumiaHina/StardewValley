#pragma once

#include <unordered_map>
#include "Game/Item.h"

namespace Game {

// 简单背包：统计物品数量
class Bag {
public:
    void add(ItemType type, int qty = 1);
    // 移除指定数量，若库存不足则移除可用的最大数量；返回是否足量移除
    bool remove(ItemType type, int qty);
    int count(ItemType type) const;

    // 为 UI 提供全部条目快照（按枚举顺序）
    std::vector<std::pair<ItemType,int>> all() const;

private:
    std::unordered_map<ItemType, int, ItemTypeHash> _counts;
};

} // namespace Game