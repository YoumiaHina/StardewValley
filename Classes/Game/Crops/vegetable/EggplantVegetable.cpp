// 茄子成熟作物：物品栏静态信息（名称/售价/恢复效果）
#include "Game/Crops/vegetable/VegetableBase.h"

namespace Game {

class EggplantVegetable : public VegetableBase {
public:
    ItemType item() const override { return ItemType::Eggplant; }
    CropType cropType() const override { return CropType::Eggplant; }
    const char* name() const override { return "Eggplant"; }
    int sellPrice() const override { return 60; }
    int energyRestore() const override { return 20; }
    int hpRestore() const override { return 4; }
};

}
