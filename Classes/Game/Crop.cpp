// 作物静态数据表与物品映射实现：
// - 每种作物的基础纹理行(baseRow16)、起始列(startCol)、各阶段天数(stageDays)、季节可用(seasons)
// - 作物类型与物品类型的双向映射（种子/成熟产物）
#include "Game/Crop.h"

namespace Game {

// 注意：BLUEBERRY 的 baseRow16 相较常规作物有所上移（贴图排布差异），
// 若需进一步校正，可在 CropDefs::baseRow16 或裁切处统一处理。
static const CropDef PARSNIP{62, 0, {1,1,1,1,1,1}, {true,false,false,false}};
static const CropDef BLUEBERRY{54, 8, {1,1,1,1,1,1,1,1}, {false,true,false,false}};
static const CropDef EGGPLANT{46, 0, {1,1,1,1,1,1,1,1}, {false,false,true,false}};
static const CropDef CORN{48, 8, {1,1,1,1,1,1,1,1}, {false,false,true,false}};
static const CropDef STRAWBERRY{26, 0, {1,1,1,1,1,1,1}, {true,false,false,false}};

// 访问指定作物的静态定义
const CropDef& CropDefs::get(CropType t) {
    switch (t) {
        case CropType::Parsnip: return PARSNIP;
        case CropType::Blueberry: return BLUEBERRY;
        case CropType::Eggplant: return EGGPLANT;
        case CropType::Corn: return CORN;
        case CropType::Strawberry: return STRAWBERRY;
        default: return PARSNIP;
    }
}

// 静态属性快捷访问
const std::vector<int>& CropDefs::stageDays(CropType t) { return get(t).stageDays; }
int CropDefs::maxStage(CropType t) { return static_cast<int>(get(t).stageDays.size()) - 1; }
int CropDefs::startCol(CropType t) { return get(t).startCol; }
int CropDefs::baseRow16(CropType t) { return get(t).baseRow16; }
bool CropDefs::isSeasonAllowed(CropType t, int seasonIndex) { return get(t).seasons[seasonIndex % 4]; }

// 作物类型 → 种子物品类型
ItemType seedItemFor(CropType t) {
    switch (t) {
        case CropType::Parsnip: return ItemType::ParsnipSeed;
        case CropType::Blueberry: return ItemType::BlueberrySeed;
        case CropType::Eggplant: return ItemType::EggplantSeed;
        case CropType::Corn: return ItemType::CornSeed;
        case CropType::Strawberry: return ItemType::StrawberrySeed;
        default: return ItemType::ParsnipSeed;
    }
}

// 作物类型 → 成熟产物物品类型
ItemType produceItemFor(CropType t) {
    switch (t) {
        case CropType::Parsnip: return ItemType::Parsnip;
        case CropType::Blueberry: return ItemType::Blueberry;
        case CropType::Eggplant: return ItemType::Eggplant;
        case CropType::Corn: return ItemType::Corn;
        case CropType::Strawberry: return ItemType::Strawberry;
        default: return ItemType::Parsnip;
    }
}

// 是否为种子类物品（用于播种判定）
bool isSeed(ItemType t) {
    return t == ItemType::ParsnipSeed || t == ItemType::BlueberrySeed || t == ItemType::EggplantSeed || t == ItemType::CornSeed || t == ItemType::StrawberrySeed;
}

// 种子物品类型 → 作物类型
CropType cropTypeFromSeed(ItemType t) {
    switch (t) {
        case ItemType::ParsnipSeed: return CropType::Parsnip;
        case ItemType::BlueberrySeed: return CropType::Blueberry;
        case ItemType::EggplantSeed: return CropType::Eggplant;
        case ItemType::CornSeed: return CropType::Corn;
        case ItemType::StrawberrySeed: return CropType::Strawberry;
        default: return CropType::Parsnip;
    }
}

}
