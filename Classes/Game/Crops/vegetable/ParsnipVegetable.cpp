// 萝卜成熟作物：物品栏静态信息（名称/售价/恢复效果）
#include "Game/Crops/vegetable/VegetableBase.h"

namespace Game {

// 萝卜成熟产物：仅提供背包内静态元数据（售价/食用恢复等）。
class ParsnipVegetable : public VegetableBase {
public:
    // 返回该物品对应的枚举类型。
    ItemType item() const override { return ItemType::Parsnip; }
    // 返回该产物来源作物类型。
    CropType cropType() const override { return CropType::Parsnip; }
    // 返回用于 UI 展示的名称。
    const char* name() const override { return "Parsnip"; }
    // 返回出售单价（用于商店卖出）。
    int sellPrice() const override { return 35; }
    // 返回食用恢复精力值。
    int energyRestore() const override { return 25; }
    // 返回食用恢复生命值。
    int hpRestore() const override { return 0; }
};

}
