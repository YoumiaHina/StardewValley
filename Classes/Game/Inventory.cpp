/**
 * Inventory implementation.
 */
#include "Game/Inventory.h"

namespace Game {

Inventory::Inventory(std::size_t slots) : _slots(slots) {}

void Inventory::setTool(std::size_t index, const Tool& tool) {
    if (index < _slots.size()) {
        _slots[index].kind = SlotKind::Tool;
        _slots[index].tool = tool;
        _slots[index].itemQty = 0;
    }
}

const Tool* Inventory::toolAt(std::size_t index) const {
    if (index < _slots.size() && _slots[index].kind == SlotKind::Tool) {
        return &_slots[index].tool;
    }
    return nullptr;
}

const ItemStack Inventory::itemAt(std::size_t index) const {
    ItemStack out;
    if (index < _slots.size() && _slots[index].kind == SlotKind::Item) {
        out.type = _slots[index].itemType;
        out.quantity = _slots[index].itemQty;
    }
    return out;
}

bool Inventory::isItem(std::size_t index) const {
    return index < _slots.size() && _slots[index].kind == SlotKind::Item;
}

bool Inventory::isEmpty(std::size_t index) const {
    return index < _slots.size() && _slots[index].kind == SlotKind::Empty;
}

int Inventory::addItems(ItemType type, int qty) {
    if (qty <= 0) return 0;
    // 1) fill existing stacks
    for (auto &s : _slots) {
        if (s.kind == SlotKind::Item && s.itemType == type && s.itemQty < ItemStack::MAX_STACK) {
            int canAdd = ItemStack::MAX_STACK - s.itemQty;
            int add = qty < canAdd ? qty : canAdd;
            s.itemQty += add;
            qty -= add;
            if (qty <= 0) return 0;
        }
    }
    // 2) create new stacks in empty slots
    for (auto &s : _slots) {
        if (s.kind == SlotKind::Empty) {
            s.kind = SlotKind::Item;
            s.itemType = type;
            int add = qty < ItemStack::MAX_STACK ? qty : ItemStack::MAX_STACK;
            s.itemQty = add;
            qty -= add;
            if (qty <= 0) return 0;
        }
    }
    // return remaining that couldn't fit
    return qty;
}

void Inventory::selectIndex(int index) {
    if (_slots.empty()) { _selected = 0; return; }
    if (index < 0) index = 0;
    if (index >= static_cast<int>(_slots.size())) index = static_cast<int>(_slots.size()) - 1;
    _selected = index;
}

void Inventory::next() {
    if (_slots.empty()) return;
    _selected = (_selected + 1) % static_cast<int>(_slots.size());
}

void Inventory::prev() {
    if (_slots.empty()) return;
    _selected = (_selected - 1);
    if (_selected < 0) _selected = static_cast<int>(_slots.size()) - 1;
}

} // namespace Game