#pragma once

#include <memory>
#include "Game/Inventory.h"
#include "Game/Item.h"
#include "Controllers/CropSystem.h"
#include "Game/Crop.h"

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

    static void fastGrowAllCrops(Controllers::CropSystem* crop) {
        if (!crop) return;
        crop->instantMatureAllCrops();
    }

    static void grantSeed(const std::shared_ptr<Inventory>& inv, Game::CropType type, int qty = 1) {
        if (!inv || qty <= 0) return;
        auto item = Game::seedItemFor(type);
        inv->addItems(item, qty);
    }

    static void grantProduce(const std::shared_ptr<Inventory>& inv, Game::CropType type, int qty = 1) {
        if (!inv || qty <= 0) return;
        auto item = Game::produceItemFor(type);
        inv->addItems(item, qty);
    }
};

}
