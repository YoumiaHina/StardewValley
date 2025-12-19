// 物品栏“成熟作物”状态接口：
// - 描述成熟产物的物品类型、名称、售卖价与食用恢复效果
// - 与种下后的 Crop 行为解耦，仅承担背包内静态信息
#pragma once

#include "Game/Item.h"
#include "Game/Crops/crop/CropBase.h"

namespace Game {

// 成熟产物物品静态信息基类：
// - 描述背包中“成熟作物产物”的名称/售价/食用恢复等元数据。
// - 协作对象：CropSystem 在收获时根据 CropType 通过映射函数生成对应 ItemType。
class VegetableBase {
public:
    virtual ~VegetableBase() = default;
    // 对应的物品枚举
    virtual ItemType item() const = 0;
    // 对应的作物类型（收获来源）
    virtual CropType cropType() const = 0;
    // 展示名称（用于UI）
    virtual const char* name() const = 0;
    // 售卖价格（商店/背包出售参考）
    virtual int sellPrice() const = 0;
    // 食用恢复精力值
    virtual int energyRestore() const = 0;
    // 食用恢复生命值
    virtual int hpRestore() const = 0;

    // 将作物类型映射为成熟产物物品类型（用于收获/掉落）。
    static ItemType produceItemFor(CropType t) {
        switch (t) {
            case CropType::Parsnip: return ItemType::Parsnip;
            case CropType::Blueberry: return ItemType::Blueberry;
            case CropType::Eggplant: return ItemType::Eggplant;
            case CropType::Corn: return ItemType::Corn;
            case CropType::Strawberry: return ItemType::Strawberry;
            default: return ItemType::Parsnip;
        }
    }
};

}
