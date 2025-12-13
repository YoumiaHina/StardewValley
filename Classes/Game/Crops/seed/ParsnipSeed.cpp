#include "Game/Crops/seed/SeedBase.h"

namespace Game {

class ParsnipSeed : public SeedBase {
public:
    ItemType item() const override { return ItemType::ParsnipSeed; }
    CropType cropType() const override { return CropType::Parsnip; }
    const char* name() const override { return "Parsnip Seed"; }
    int price() const override { return 25; }
};

}
