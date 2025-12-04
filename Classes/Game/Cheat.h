#pragma once

#include <memory>
#include "Game/Inventory.h"
#include "Game/Item.h"
#include "Controllers/IMapController.h"

namespace Game {

struct Cheat {
    static void grantBasic(const std::shared_ptr<Inventory>& inv) {
        if (!inv) return;
        for (auto t : { ItemType::Wood, ItemType::Stone, ItemType::Fiber, ItemType::Chest,
                        ItemType::Parsnip,
                        ItemType::ParsnipSeed, ItemType::BlueberrySeed, ItemType::EggplantSeed }) {
            inv->addItems(t, 99);
        }
    }

    static void fastGrowAllCrops(Controllers::IMapController* map) {
        if (!map) return;
        map->instantMatureAllCrops();
    }
};

}

