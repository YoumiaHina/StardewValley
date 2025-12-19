// 玉米：倒数阶段可采摘一次，成熟阶段为“已采摘”占位；支持每日回生退回倒数阶段
#include "Game/Crops/crop/CropBase.h"

namespace Game {

// 玉米作物行为：提供静态定义与物品映射，不直接参与 CropSystem 的生命周期管理。
class CornCrop : public CropBase {
public:
    // 初始化该作物的静态定义与映射（种子/产物/季节/阶段）。
    CornCrop() {
        def_.baseRow16 = 48;
        def_.startCol = 8;
        def_.stageDays = {1, 1, 1, 1, 1, 1, 1, 1};
        def_.seasons = {false, false, true, false};
        seedItem_ = ItemType::CornSeed;
        produceItem_ = ItemType::Corn;
        regrow_ = true;
    }

    // 返回作物类型枚举，用于被 CropDefs/CropSystem 索引。
    CropType cropType() const override { return CropType::Corn; }
};

// 获取玉米作物行为单例：由 CropDefs 统一转发引用。
const CropBase& cornCropBehavior() {
    static CornCrop inst;
    return inst;
}

}
