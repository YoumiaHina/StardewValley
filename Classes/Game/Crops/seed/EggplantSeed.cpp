#include "Game/Crops/seed/SeedBase.h"

namespace Game {

class EggplantSeed : public SeedBase {
public:
    ItemType item() const override { return ItemType::EggplantSeed; }
    CropType cropType() const override { return CropType::Eggplant; }
    const char* name() const override { return "Eggplant Seed"; }
    int price() const override { return 25; }
};

}
