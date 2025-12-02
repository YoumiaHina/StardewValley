#include "Game/Bag.h"

namespace Game {

void Bag::add(ItemType type, int qty) {
    if (qty <= 0) return;
    _counts[type] += qty;
}

int Bag::count(ItemType type) const {
    auto it = _counts.find(type);
    return it == _counts.end() ? 0 : it->second;
}

std::vector<std::pair<ItemType,int>> Bag::all() const {
    std::vector<std::pair<ItemType,int>> out;
    out.reserve(3);
    for (auto t : { ItemType::Wood, ItemType::Stone, ItemType::Fiber }) {
        out.emplace_back(t, count(t));
    }
    return out;
}

} // namespace Game