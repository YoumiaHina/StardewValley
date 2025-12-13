#include "Game/Crop.h"

namespace Game {

static const CropDef PARSNIP{62, 0, {1,1,1,1,1,1}, {true,false,false,false}};
static const CropDef BLUEBERRY{54, 8, {1,1,1,1,1,1,1,1}, {false,true,false,false}};
static const CropDef EGGPLANT{46, 0, {1,1,1,1,1,1,1,1}, {false,false,true,false}};
static const CropDef CORN{48, 8, {1,1,1,1,1,1,1,1}, {false,false,true,false}};
static const CropDef STRAWBERRY{26, 0, {1,1,1,1,1,1,1}, {true,false,false,false}};

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

const std::vector<int>& CropDefs::stageDays(CropType t) { return get(t).stageDays; }
int CropDefs::maxStage(CropType t) { return static_cast<int>(get(t).stageDays.size()) - 1; }
int CropDefs::startCol(CropType t) { return get(t).startCol; }
int CropDefs::baseRow16(CropType t) { return get(t).baseRow16; }
bool CropDefs::isSeasonAllowed(CropType t, int seasonIndex) { return get(t).seasons[seasonIndex % 4]; }

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

bool isSeed(ItemType t) {
    return t == ItemType::ParsnipSeed || t == ItemType::BlueberrySeed || t == ItemType::EggplantSeed || t == ItemType::CornSeed || t == ItemType::StrawberrySeed;
}

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
