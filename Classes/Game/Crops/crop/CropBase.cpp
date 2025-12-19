#include "Game/Crops/crop/CropBase.h"
#include "Game/Crops/seed/SeedBase.h"
#include "Game/Crops/vegetable/VegetableBase.h"

#include <algorithm>

namespace Game {

// 各作物类型的行为单例：由对应的 *Crop.cpp 提供。
const CropBase& parsnipCropBehavior();
const CropBase& blueberryCropBehavior();
const CropBase& eggplantCropBehavior();
const CropBase& cornCropBehavior();
const CropBase& strawberryCropBehavior();

// 根据作物类型返回对应的行为单例。
static const CropBase& cropInfoFor(CropType t) {
    switch (t) {
        case CropType::Parsnip: return parsnipCropBehavior();
        case CropType::Blueberry: return blueberryCropBehavior();
        case CropType::Eggplant: return eggplantCropBehavior();
        case CropType::Corn: return cornCropBehavior();
        case CropType::Strawberry: return strawberryCropBehavior();
        default: return parsnipCropBehavior();
    }
}

// 从行为单例获取静态定义。
const CropDef& CropDefs::get(CropType t) {
    return cropInfoFor(t).def();
}

// 读取各类静态字段的便捷接口。
const std::vector<int>& CropDefs::stageDays(CropType t) { return get(t).stageDays; }
int CropDefs::maxStage(CropType t) { return static_cast<int>(get(t).stageDays.size()) - 1; }
int CropDefs::startCol(CropType t) { return get(t).startCol; }
int CropDefs::baseRow16(CropType t) { return get(t).baseRow16; }
bool CropDefs::isSeasonAllowed(CropType t, int seasonIndex) { return get(t).seasons[seasonIndex % 4]; }
bool CropDefs::isRegrow(CropType t) { return cropInfoFor(t).regrow(); }

// 将作物类型映射为种子物品：委托给 SeedBase 的静态映射表。
ItemType seedItemFor(CropType t) {
    return SeedBase::seedItemFor(t);
}

// 将作物类型映射为产物物品：委托给 VegetableBase 的静态映射表。
ItemType produceItemFor(CropType t) {
    return VegetableBase::produceItemFor(t);
}

// 判断物品是否为种子：委托给 SeedBase。
bool isSeed(ItemType t) {
    return SeedBase::isSeed(t);
}

// 将种子物品类型映射回作物类型：委托给 SeedBase。
CropType cropTypeFromSeed(ItemType t) {
    return SeedBase::cropTypeFromSeed(t);
}

// 计算作物贴图下半块裁切矩形：按 16px 单元裁切，y 以纹理左下角为原点。
cocos2d::Rect cropRectBottomHalf(CropType t, int stage, float textureHeight) {
    int bottomRow16 = CropDefs::baseRow16(t);
    int maxStage = CropDefs::maxStage(t);
    int s = std::max(0, std::min(stage, maxStage));
    int col = CropDefs::startCol(t) + s;
    float y = textureHeight - (bottomRow16 + 1) * 16.0f;
    return cocos2d::Rect(col * 16.0f, y, 16.0f, 16.0f);
}

// 计算作物贴图上半块裁切矩形：位于下半块上方一行（16px）。
cocos2d::Rect cropRectTopHalf(CropType t, int stage, float textureHeight) {
    int bottomRow16 = CropDefs::baseRow16(t);
    int topRow16 = bottomRow16 + 1;
    int maxStage = CropDefs::maxStage(t);
    int s = std::max(0, std::min(stage, maxStage));
    int col = CropDefs::startCol(t) + s;
    float y = textureHeight - (topRow16 + 1) * 16.0f;
    return cocos2d::Rect(col * 16.0f, y, 16.0f, 16.0f);
}

}
