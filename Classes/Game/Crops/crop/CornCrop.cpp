// 玉米：倒数阶段可采摘一次，成熟阶段为“已采摘”占位；支持每日回生退回倒数阶段
#include "Game/Crops/crop/CropBase.h"

namespace Game {

class CornCrop : public CropBase {
public:
    CornCrop() {
        def_.baseRow16 = 48;
        def_.startCol = 8;
        def_.stageDays = {1, 1, 1, 1, 1, 1, 1, 1};
        def_.seasons = {false, false, true, false};
        seedItem_ = ItemType::CornSeed;
        produceItem_ = ItemType::Corn;
        regrow_ = true;
    }

    CropType cropType() const override { return CropType::Corn; }
};

const CropBase& cornCropBehavior() {
    static CornCrop inst;
    return inst;
}

}
