#include "Game/Crops/seed/SeedBase.h"

namespace Game {

class CornSeed : public SeedBase {
public:
    ItemType item() const override { return ItemType::CornSeed; }
    CropType cropType() const override { return CropType::Corn; }
    const char* name() const override { return "Corn Seed"; }
    int price() const override { return 25; }
};

}
