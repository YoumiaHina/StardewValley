// 蓝莓：倒数阶段可采摘一次，成熟阶段为“已采摘”占位；支持每日回生退回倒数阶段
#include "Game/Crops/crop/CropBase.h"

namespace Game {

class BlueberryCrop : public RegrowCropBase {
};

const CropBase& blueberryCropBehavior() {
    static BlueberryCrop inst;
    return inst;
}

}
