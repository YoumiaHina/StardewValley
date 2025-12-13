#include "Game/Crops/vegetable/VegetableBase.h"

namespace Game {

class StrawberryVegetable : public VegetableBase {
public:
    ItemType item() const override { return ItemType::Strawberry; }
    CropType cropType() const override { return CropType::Strawberry; }
    const char* name() const override { return "Strawberry"; }
    int sellPrice() const override { return 120; }
    int energyRestore() const override { return 50; }
    int hpRestore() const override { return 2; }
};

}
