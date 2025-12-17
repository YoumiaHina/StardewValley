// 萝卜：一次性成熟，可收获即移除，不回生
#include "Game/Crops/crop/CropBase.h"

namespace Game {

class ParsnipCrop : public CropBase {
public:
    ParsnipCrop() {
        def_.baseRow16 = 62;
        def_.startCol = 0;
        def_.stageDays = {1, 1, 1, 1, 1, 1};
        def_.seasons = {true, false, false, false};
        seedItem_ = ItemType::ParsnipSeed;
        produceItem_ = ItemType::Parsnip;
    }

    CropType cropType() const override { return CropType::Parsnip; }
};

const CropBase& parsnipCropBehavior() {
    static ParsnipCrop inst;
    return inst;
}

}
