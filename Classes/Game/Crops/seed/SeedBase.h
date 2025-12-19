// 物品栏“种子”状态接口：
// - 描述种子的物品类型、对应作物类型与价格等元数据
// - 与种下后的 CropBase 行为解耦，仅承担背包内静态信息
#pragma once

#include "Game/Item.h"
#include "Game/Crops/crop/CropBase.h"

namespace Game {

// 种子物品静态信息基类：
// - 描述背包中“种子”的名称/价格/对应作物类型等元数据。
// - 协作对象：CropBase 负责作物静态定义与行为；播种逻辑通过 CropType 与 ItemType 映射协作。
class SeedBase {
public:
    virtual ~SeedBase() = default;
    // 对应的物品枚举
    virtual ItemType item() const = 0;
    // 对应的作物类型（播种后转为该类型的 Crop）
    virtual CropType cropType() const = 0;
    // 展示名称（用于UI）
    virtual const char* name() const = 0;
    // 购买价格或物品本身定价（具体由商店策略决定）
    virtual int price() const = 0;

    // 将作物类型映射为种子物品类型（用于商店/作弊发放）。
    static ItemType seedItemFor(CropType t) {
        switch (t) {
            case CropType::Parsnip: return ItemType::ParsnipSeed;
            case CropType::Blueberry: return ItemType::BlueberrySeed;
            case CropType::Eggplant: return ItemType::EggplantSeed;
            case CropType::Corn: return ItemType::CornSeed;
            case CropType::Strawberry: return ItemType::StrawberrySeed;
            default: return ItemType::ParsnipSeed;
        }
    }

    // 判断某个物品类型是否为种子（用于播种判定/喂鸡判定等）。
    static bool isSeed(ItemType t) {
        switch (t) {
            case ItemType::ParsnipSeed:
            case ItemType::BlueberrySeed:
            case ItemType::EggplantSeed:
            case ItemType::CornSeed:
            case ItemType::StrawberrySeed:
                return true;
            default:
                return false;
        }
    }

    // 将种子物品类型映射回作物类型（用于从背包解析播种目标）。
    static CropType cropTypeFromSeed(ItemType t) {
        switch (t) {
            case ItemType::ParsnipSeed: return CropType::Parsnip;
            case ItemType::BlueberrySeed: return CropType::Blueberry;
            case ItemType::EggplantSeed: return CropType::Eggplant;
            case ItemType::CornSeed: return CropType::Corn;
            case ItemType::StrawberrySeed: return CropType::Strawberry;
            default: return CropType::Parsnip;
        }
    }
};

}
