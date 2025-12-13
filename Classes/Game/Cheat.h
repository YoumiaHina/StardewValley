// 辅助调试/作弊接口：
// - 快速发放基础物资、使作物瞬间成熟、发放种子/产物
// - 仅通过公开的系统/映射接口协作，不直接修改内部状态
#pragma once

#include <memory>
#include "Game/Inventory.h"
#include "Game/Item.h"
#include "Controllers/Systems/CropSystem.h"
#include "Game/Crop.h"

namespace Game {

struct Cheat {
    static void grantBasic(const std::shared_ptr<Inventory>& inv) {
        if (!inv) return;
        for (auto t : { ItemType::Wood, ItemType::Stone, ItemType::Fiber, ItemType::Chest,
                        ItemType::Parsnip }) {
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
