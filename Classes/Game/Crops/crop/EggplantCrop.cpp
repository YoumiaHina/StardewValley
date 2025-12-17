// 茄子：倒数阶段可采摘一次，成熟阶段为“已采摘”占位；支持每日回生退回倒数阶段
#include "Game/Crops/crop/CropBase.h"

namespace Game {

class EggplantCrop : public CropBase {
public:
    EggplantCrop() {
        def_.baseRow16 = 46;
        def_.startCol = 0;
        def_.stageDays = {1, 1, 1, 1, 1, 1, 1, 1};
        def_.seasons = {false, false, true, false};
        seedItem_ = ItemType::EggplantSeed;
        produceItem_ = ItemType::Eggplant;
        regrow_ = true;
    }

    CropType cropType() const override { return CropType::Eggplant; }
};

const CropBase& eggplantCropBehavior() {
    static EggplantCrop inst;
    return inst;
}

}
