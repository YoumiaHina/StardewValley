// 萝卜：一次性成熟，可收获即移除，不回生
#include "Game/Crops/crop/CropBase.h"

namespace Game {

// 萝卜作物行为：提供静态定义与物品映射，不直接参与 CropSystem 的生命周期管理。
class ParsnipCrop : public CropBase {
public:
    // 初始化该作物的静态定义与映射（种子/产物/季节/阶段）。
    ParsnipCrop() {
        def_.baseRow16 = 62;
        def_.startCol = 0;
        def_.stageDays = {1, 1, 1, 1, 1, 1};
        def_.seasons = {true, false, false, false};
        seedItem_ = ItemType::ParsnipSeed;
        produceItem_ = ItemType::Parsnip;
    }

    // 返回作物类型枚举，用于被 CropDefs/CropSystem 索引。
    CropType cropType() const override { return CropType::Parsnip; }
};

// 获取萝卜作物行为单例：由 CropDefs 统一转发引用。
const CropBase& parsnipCropBehavior() {
    static ParsnipCrop inst;
    return inst;
}

}
