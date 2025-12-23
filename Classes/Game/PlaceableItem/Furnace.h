#pragma once

#include "cocos2d.h"
#include "Game/GameConfig.h"
#include "Game/PlaceableItem/PlaceableItemBase.h"
#include "Game/Item.h"

namespace Game {

// Furnace：可放置的熔炉实体，用于把矿石与燃料熔炼成锭。
// - 职责：保存熔炉在世界中的位置、当前矿石类型、剩余熔炼时间与掉落偏移。
// - 协作：由 FurnaceController 维护列表/驱动计时与掉落；WorldState 负责持久化。
// - 约束：不包含 UI 或背包逻辑，仅承担“熔炉状态”数据结构角色。
struct Furnace : public PlaceableItemBase {
    cocos2d::Vec2 dropOffset;         // 熔炼完成后掉落物品的偏移位置
    ItemType oreType = ItemType::CopperGrain; // 当前正在熔炼的矿石类型
    float remainingSeconds = 0.0f;    // 剩余熔炼时间，<=0 表示空闲

    // 返回熔炉的占用矩形（1x2 格，用于放置/渲染）。
    cocos2d::Rect placeRect() const override;
    // 返回熔炉的碰撞矩形（通常只用下半部分）。
    cocos2d::Rect collisionRect() const override;
    // 每个地图区域允许存在的最大熔炉数量。
    int maxPerArea() const override { return 200; }
};

// FurnaceRecipe：一条熔炉配方定义。
// - ore：矿石类型；fuel：燃料类型；output：产出的锭类型。
// - oreCount/fuelCount：一次熔炼消耗的数量；seconds：所需时间。
struct FurnaceRecipe {
    ItemType ore;
    ItemType fuel;
    ItemType output;
    int oreCount;
    int fuelCount;
    float seconds;
};

// 根据矿石类型返回对应熔炉配方；若找不到，返回一个“无效配方”占位。
FurnaceRecipe furnaceRecipeFor(ItemType ore);
// 判断配是否有效（数量>0 且时间>0），常配合 furnaceRecipeFor 使用。
bool isValidFurnaceRecipe(const FurnaceRecipe& r);

} // namespace Game
