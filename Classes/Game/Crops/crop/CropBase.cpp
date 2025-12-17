#include "Game/Crops/crop/CropBase.h"
#include "Game/Crops/seed/SeedBase.h"
#include "Game/Crops/vegetable/VegetableBase.h"

#include <algorithm>

namespace Game {

const CropBase& parsnipCropBehavior();
const CropBase& blueberryCropBehavior();
const CropBase& eggplantCropBehavior();
const CropBase& cornCropBehavior();
const CropBase& strawberryCropBehavior();

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

const CropDef& CropDefs::get(CropType t) {
    return cropInfoFor(t).def();
}

const std::vector<int>& CropDefs::stageDays(CropType t) { return get(t).stageDays; }
int CropDefs::maxStage(CropType t) { return static_cast<int>(get(t).stageDays.size()) - 1; }
int CropDefs::startCol(CropType t) { return get(t).startCol; }
int CropDefs::baseRow16(CropType t) { return get(t).baseRow16; }
bool CropDefs::isSeasonAllowed(CropType t, int seasonIndex) { return get(t).seasons[seasonIndex % 4]; }
bool CropDefs::isRegrow(CropType t) { return cropInfoFor(t).regrow(); }

ItemType seedItemFor(CropType t) {
    return SeedBase::seedItemFor(t);
}

ItemType produceItemFor(CropType t) {
    return VegetableBase::produceItemFor(t);
}

bool isSeed(ItemType t) {
    return SeedBase::isSeed(t);
}

CropType cropTypeFromSeed(ItemType t) {
    return SeedBase::cropTypeFromSeed(t);
}

cocos2d::Rect cropRectBottomHalf(CropType t, int stage, float textureHeight) {
    int bottomRow16 = CropDefs::baseRow16(t);
    int maxStage = CropDefs::maxStage(t);
    int s = std::max(0, std::min(stage, maxStage));
    int col = CropDefs::startCol(t) + s;
    float y = textureHeight - (bottomRow16 + 1) * 16.0f;
    return cocos2d::Rect(col * 16.0f, y, 16.0f, 16.0f);
}

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
