#include "Game/Crops/vegetable/VegetableBase.h"

namespace Game {

class BlueberryVegetable : public VegetableBase {
public:
    ItemType item() const override { return ItemType::Blueberry; }
    CropType cropType() const override { return CropType::Blueberry; }
    const char* name() const override { return "Blueberry"; }
    int sellPrice() const override { return 50; }
    int energyRestore() const override { return 25; }
    int hpRestore() const override { return 2; }
};

}
