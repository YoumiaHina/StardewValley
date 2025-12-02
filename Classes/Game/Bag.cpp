#include "Game/Bag.h"

namespace Game {

void Bag::add(ItemType type, int qty) {
    if (qty <= 0) return;
    _counts[type] += qty;
}

bool Bag::remove(ItemType type, int qty) {
    if (qty <= 0) return true;
    auto it = _counts.find(type);
    if (it == _counts.end()) return false;
    int have = it->second;
    if (have <= 0) return false;
    int take = std::min(have, qty);
    it->second = have - take;
    if (it->second <= 0) {
        _counts.erase(it);
    }
    return take == qty;
}

int Bag::count(ItemType type) const {
    auto it = _counts.find(type);
    return it == _counts.end() ? 0 : it->second;
}

std::vector<std::pair<ItemType,int>> Bag::all() const {
    std::vector<std::pair<ItemType,int>> out;
    out.reserve(_counts.size());
    for (const auto& kv : _counts) {
        if (kv.second > 0) {
            out.emplace_back(kv.first, kv.second);
        }
    }
    // 保持稳定顺序：按枚举值排序
    std::sort(out.begin(), out.end(), [](const std::pair<ItemType,int>& a, const std::pair<ItemType,int>& b){
        return static_cast<int>(a.first) < static_cast<int>(b.first);
    });
    return out;
}

} // namespace Game