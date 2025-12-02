/**
 * Inventory: Stardew-like unified slots for tools & items.
 */
#pragma once

#include <vector>
#include <cstddef>
#include <utility>
#include "Game/Tool.h"
#include "Game/Item.h"

namespace Game {

enum class SlotKind { Empty, Tool, Item };

struct ItemStack {
    ItemType type = ItemType::Wood;
    int quantity = 0;
    static const int MAX_STACK = 999;
};

struct Slot {
    SlotKind kind = SlotKind::Empty;
    Tool tool;            // valid when kind == Tool
    ItemType itemType = ItemType::Wood; // valid when kind == Item
    int itemQty = 0;      // valid when kind == Item
};

class Inventory {
public:
    explicit Inventory(std::size_t slots);

    std::size_t size() const { return _slots.size(); }

    // tools
    void setTool(std::size_t index, const Tool& tool);
    const Tool* toolAt(std::size_t index) const;

    // items
    const ItemStack itemAt(std::size_t index) const;
    bool isItem(std::size_t index) const;
    bool isEmpty(std::size_t index) const;
    // add items into inventory, stacking first; returns remaining qty not fitted
    int addItems(ItemType type, int qty);

    // 消耗当前选中槽位中的物品（当选中为物品槽）
    // 返回是否成功消耗至少 1 个
    bool consumeSelectedItem(int qty = 1);

    int selectedIndex() const { return _selected; }
    void selectIndex(int index);
    void next();
    void prev();

    const Tool* selectedTool() const { return toolAt(static_cast<std::size_t>(_selected)); }
    SlotKind selectedKind() const { return _slots.empty() ? SlotKind::Empty : _slots[_selected].kind; }
    Slot const& selectedSlot() const { return _slots[_selected]; }

private:
    std::vector<Slot> _slots;
    int _selected = 0;
};

} // namespace Game