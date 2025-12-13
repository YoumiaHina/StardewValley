#include "Game/Crops/seed/SeedBase.h"

namespace Game {

class BlueberrySeed : public SeedBase {
public:
    ItemType item() const override { return ItemType::BlueberrySeed; }
    CropType cropType() const override { return CropType::Blueberry; }
    const char* name() const override { return "Blueberry Seed"; }
    int price() const override { return 25; }
};

}
