#pragma once

#include <memory>
#include "Game/Inventory.h"
#include "Game/Item.h"

namespace Game {

struct Cheat {
    static void grantBasic(const std::shared_ptr<Inventory>& inv) {
        if (!inv) return;
        for (auto t : { ItemType::Wood, ItemType::Stone, ItemType::Fiber, ItemType::Chest, ItemType::Parsnip, ItemType::ParsnipSeed }) {
            inv->addItems(t, 99);
        }
    }
};

}

