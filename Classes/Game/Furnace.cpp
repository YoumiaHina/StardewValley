#include "Game/Furnace.h"

using namespace cocos2d;

namespace Game {

// 返回熔炉在世界坐标中的占用矩形（宽度 1 格，高度 2 格）。
Rect Furnace::placeRect() const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return PlaceableItemBase::standard1x2PlaceRect(pos, s);
}

// 返回用于角色/环境碰撞的矩形，仅覆盖熔炉底半部分。
Rect Furnace::collisionRect() const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return PlaceableItemBase::standardBottomHalfCollisionRect(pos, s);
}

// 根据矿石类型返回熔炉配方：
// - 目前支持铜/铁/金三种矿石，对应固定耗材与时长。
// - default 分支返回一个“无效配方”（数量=0），配合 isValidFurnaceRecipe 过滤。
FurnaceRecipe furnaceRecipeFor(ItemType ore) {
    switch (ore) {
        case ItemType::CopperGrain:
            return FurnaceRecipe{ ItemType::CopperGrain, ItemType::Coal, ItemType::CopperIngot, 5, 1, 30.0f };
        case ItemType::IronGrain:
            return FurnaceRecipe{ ItemType::IronGrain, ItemType::Coal, ItemType::IronIngot, 5, 1, 30.0f };
        case ItemType::GoldGrain:
            return FurnaceRecipe{ ItemType::GoldGrain, ItemType::Coal, ItemType::GoldIngot, 5, 1, 30.0f };
        default:
            // 注意：这里的 Wood 类型仅作为“无效占位”，真正逻辑依赖 isValidFurnaceRecipe 过滤；数值为 0 表示不可用。
            return FurnaceRecipe{ ItemType::Wood, ItemType::Wood, ItemType::Wood, 0, 0, 0.0f };
    }
}

// 判断配方是否有效：数量>0 且时间>0。
bool isValidFurnaceRecipe(const FurnaceRecipe& r) {
    return r.oreCount > 0 && r.fuelCount > 0 && r.seconds > 0.0f;
}

} // namespace Game
