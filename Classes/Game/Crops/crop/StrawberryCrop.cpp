// 草莓：倒数阶段可采摘一次，成熟阶段为“已采摘”占位；支持每日回生退回倒数阶段
#include "Game/Crops/crop/CropBase.h"

namespace Game {

// 草莓作物行为：提供静态定义与物品映射，不直接参与 CropSystem 的生命周期管理。
class StrawberryCrop : public CropBase {
public:
    // 初始化该作物的静态定义与映射（种子/产物/季节/阶段）。
    StrawberryCrop() {
        def_.baseRow16 = 26;
        def_.startCol = 0;
        def_.stageDays = {1, 1, 1, 1, 1, 1, 1, 1};
        def_.seasons = {true, false, false, false};
        seedItem_ = ItemType::StrawberrySeed;
        produceItem_ = ItemType::Strawberry;
        regrow_ = true;
    }

    // 返回作物类型枚举，用于被 CropDefs/CropSystem 索引。
    CropType cropType() const override { return CropType::Strawberry; }
};

// 获取草莓作物行为单例：由 CropDefs 统一转发引用。
const CropBase& strawberryCropBehavior() {
    static StrawberryCrop inst;
    return inst;
}

}
