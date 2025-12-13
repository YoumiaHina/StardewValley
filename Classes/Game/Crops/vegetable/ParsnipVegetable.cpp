// 萝卜成熟作物：物品栏静态信息（名称/售价/恢复效果）
#include "Game/Crops/vegetable/VegetableBase.h"

namespace Game {

class ParsnipVegetable : public VegetableBase {
public:
    ItemType item() const override { return ItemType::Parsnip; }
    CropType cropType() const override { return CropType::Parsnip; }
    const char* name() const override { return "Parsnip"; }
    int sellPrice() const override { return 35; }
    int energyRestore() const override { return 25; }
    int hpRestore() const override { return 0; }
};

}
