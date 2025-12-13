#include "Game/Crops/vegetable/VegetableBase.h"

namespace Game {

class CornVegetable : public VegetableBase {
public:
    ItemType item() const override { return ItemType::Corn; }
    CropType cropType() const override { return CropType::Corn; }
    const char* name() const override { return "Corn"; }
    int sellPrice() const override { return 50; }
    int energyRestore() const override { return 25; }
    int hpRestore() const override { return 2; }
};

}
