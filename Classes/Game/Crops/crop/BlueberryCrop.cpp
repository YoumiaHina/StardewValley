// 蓝莓：倒数阶段可采摘一次，成熟阶段为“已采摘”占位；支持每日回生退回倒数阶段
#include "Game/Crops/crop/CropBase.h"

namespace Game {

class BlueberryCrop : public CropBase {
public:
    BlueberryCrop() {
        def_.baseRow16 = 54;
        def_.startCol = 8;
        def_.stageDays = {1, 1, 1, 1, 1, 1, 1, 1};
        def_.seasons = {false, true, false, false};
        seedItem_ = ItemType::BlueberrySeed;
        produceItem_ = ItemType::Blueberry;
        regrow_ = true;
    }

    CropType cropType() const override { return CropType::Blueberry; }
};

const CropBase& blueberryCropBehavior() {
    static BlueberryCrop inst;
    return inst;
}

}
