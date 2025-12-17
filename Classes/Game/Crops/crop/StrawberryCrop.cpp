// 草莓：倒数阶段可采摘一次，成熟阶段为“已采摘”占位；支持每日回生退回倒数阶段
#include "Game/Crops/crop/CropBase.h"

namespace Game {

class StrawberryCrop : public CropBase {
public:
    StrawberryCrop() {
        def_.baseRow16 = 26;
        def_.startCol = 0;
        def_.stageDays = {1, 1, 1, 1, 1, 1, 1, 1};
        def_.seasons = {true, false, false, false};
        seedItem_ = ItemType::StrawberrySeed;
        produceItem_ = ItemType::Strawberry;
        regrow_ = true;
    }

    CropType cropType() const override { return CropType::Strawberry; }
};

const CropBase& strawberryCropBehavior() {
    static StrawberryCrop inst;
    return inst;
}

}
