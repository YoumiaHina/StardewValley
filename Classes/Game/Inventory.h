#pragma once

#include <vector>
#include <cstddef>
#include <utility>
#include <memory>
#include "Game/Tool/ToolBase.h"
#include "Game/Item.h"

namespace Game {

// SlotKind：背包槽位类型枚举。
// - Empty：空槽，不存放任何数据；
// - Tool：工具槽，保存一个 ToolBase 智能指针；
// - Item：物品槽，保存某种物品及其数量。
enum class SlotKind { Empty, Tool, Item };

// ItemStack：一种物品及其数量的简单封装。
// - type：物品类型；
// - quantity：物品数量；
// - MAX_STACK：单个槽位允许堆叠的最大数量。
struct ItemStack {
    ItemType type = ItemType::Wood;
    int quantity = 0;
    static const int MAX_STACK = 999;
};

// Slot：背包中的单个槽位。
// - kind：当前槽位类型（空/工具/物品）；
// - tool：当 kind 为 Tool 时指向具体工具实例；
// - itemType/itemQty：当 kind 为 Item 时表示物品种类与数量。
struct Slot {
    SlotKind kind = SlotKind::Empty;
    std::shared_ptr<ToolBase> tool;           
    ItemType itemType = ItemType::Wood; 
    int itemQty = 0;      
};

// Inventory：玩家与系统使用的通用背包容器。
// - 内部使用 Slot 向量顺序存储每个格子的状态；
// - 提供添加/移除物品、放入/取出工具、计数与查询等接口；
// - 通过 selectedIndex 记录当前选中的槽位，供 UI 与控制器协作。
class Inventory {
public:
    explicit Inventory(std::size_t slots);

    std::size_t size() const { return _slots.size(); }

    void setTool(std::size_t index, std::shared_ptr<ToolBase> tool);
    const ToolBase* toolAt(std::size_t index) const;
    ToolBase* toolAtMutable(std::size_t index);

    const ItemStack itemAt(std::size_t index) const;
    bool isItem(std::size_t index) const;
    bool isEmpty(std::size_t index) const;
    bool isTool(std::size_t index) const;

    int addItems(ItemType type, int qty);

    int countItems(ItemType type) const;
    
    bool removeItems(ItemType type, int qty);

    // 消耗当前选中槽位中的物品（当选中为物品槽）
    // 返回是否成功消耗至少 1 个
    bool consumeSelectedItem(int qty = 1);

    int selectedIndex() const { return _selected; }
    void selectIndex(int index);
    void next();
    void prev();

    ToolBase* selectedTool() { return toolAtMutable(static_cast<std::size_t>(_selected)); }
    SlotKind selectedKind() const { return _slots.empty() ? SlotKind::Empty : _slots[_selected].kind; }
    Slot const& selectedSlot() const { return _slots[_selected]; }
    bool addOneItemToSlot(std::size_t index, ItemType type);
    bool removeOneItemFromSlot(std::size_t index);
    bool clearSlot(std::size_t index);

private:
    std::vector<Slot> _slots;
    int _selected = 0;
};

} // namespace Game
