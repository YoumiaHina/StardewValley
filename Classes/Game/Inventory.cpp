/**
 * Inventory implementation.
 */
#include "Game/Inventory.h"

namespace Game {

Inventory::Inventory(std::size_t slots) : _slots(slots) {}

void Inventory::setSlot(std::size_t index, const Tool& tool) {
    if (index < _slots.size()) {
        _slots[index] = tool;
    }
}

const Tool* Inventory::getSlot(std::size_t index) const {
    if (index < _slots.size()) {
        return &_slots[index];
    }
    return nullptr;
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