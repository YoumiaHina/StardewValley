#include "Game/Recipe/RecipeBase.h"
#include "Game/Inventory.h"

namespace Game {

bool RecipeBase::canCraft(const Inventory& inv) const {
    const auto& req = ingredients();
    for (const auto& it : req) {
        if (it.qty <= 0) {
            return false;
        }
        int have = inv.countItems(it.type);
        if (have < it.qty) {
            return false;
        }
    }
    return true;
}

bool RecipeBase::craft(Inventory& inv) const {
    if (!canCraft(inv)) {
        return false;
    }
    const auto& req = ingredients();
    for (const auto& it : req) {
        if (!inv.removeItems(it.type, it.qty)) {
            return false;
        }
    }
    int leftover = inv.addItems(outputType(), outputQty());
    return leftover == 0;
}

} // namespace Game

