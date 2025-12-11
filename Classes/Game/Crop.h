#pragma once

#include <vector>
#include <array>
#include "Game/Item.h"

namespace Game {

enum class CropType { Parsnip, Blueberry, Eggplant, Corn, Strawberry };

// 作物实例：记录所在格子、类型与当前生长状态
struct Crop {
    int c = 0;
    int r = 0;
    CropType type = CropType::Parsnip;
    int stage = 0;
    int progress = 0;
    int maxStage = 0;
    bool wateredToday = false;
};

// 作物静态定义：纹理行(16像素为单位)、起始列与各阶段所需天数（左下角为(0,0)）
struct CropDef {
    int baseRow16 = 0;
    int startCol = 0;
    std::vector<int> stageDays;
    std::array<bool,4> seasons;
};

// 作物定义表：集中访问每种作物的静态属性
class CropDefs {
public:
    static const CropDef& get(CropType t) {
        static const CropDef parsnip{62, 0, {1,1,1,1,1,1}, {true,false,false,false}};
        static const CropDef blueberry{55, 8, {1,1,1,1,1,1,1,1}, {false,true,false,false}};
        static const CropDef eggplant{46, 0, {1,1,1,1,1,1,1,1}, {false,false,true,false}};
        static const CropDef corn{48, 8, {1,1,1,1,1,1,1,1}, {false,false,true,false}};
        static const CropDef strawberry{26, 0, {1,1,1,1,1,1,1}, {true,false,false,false}};
        switch (t) {
            case CropType::Parsnip: return parsnip;
            case CropType::Blueberry: return blueberry;
            case CropType::Eggplant: return eggplant;
            case CropType::Corn: return corn;
            case CropType::Strawberry: return strawberry;
            default: return parsnip;
        }
    }
    static const std::vector<int>& stageDays(CropType t) { return get(t).stageDays; }
    static int maxStage(CropType t) { return static_cast<int>(get(t).stageDays.size()) - 1; }
    static int startCol(CropType t) { return get(t).startCol; }
    static int baseRow16(CropType t) { return get(t).baseRow16; }
    static bool isSeasonAllowed(CropType t, int seasonIndex) { return get(t).seasons[seasonIndex % 4]; }
};

// 根据作物类型返回对应的“种子”物品类型
inline Game::ItemType seedItemFor(CropType t) {
    switch (t) {
        case CropType::Parsnip: return Game::ItemType::ParsnipSeed;
        case CropType::Blueberry: return Game::ItemType::BlueberrySeed;
        case CropType::Eggplant: return Game::ItemType::EggplantSeed;
        case CropType::Corn: return Game::ItemType::CornSeed;
        case CropType::Strawberry: return Game::ItemType::StrawberrySeed;
        default: return Game::ItemType::ParsnipSeed;
    }
}

// 根据作物类型返回对应的“收获产物”物品类型
inline Game::ItemType produceItemFor(CropType t) {
    switch (t) {
        case CropType::Parsnip: return Game::ItemType::Parsnip;
        case CropType::Blueberry: return Game::ItemType::Blueberry;
        case CropType::Eggplant: return Game::ItemType::Eggplant;
        case CropType::Corn: return Game::ItemType::Corn;
        case CropType::Strawberry: return Game::ItemType::Strawberry;
        default: return Game::ItemType::Parsnip;
    }
}

// 是否为“种子”类物品（用于播种判定）
inline bool isSeed(Game::ItemType t) {
    return t == Game::ItemType::ParsnipSeed || t == Game::ItemType::BlueberrySeed || t == Game::ItemType::EggplantSeed || t == Game::ItemType::CornSeed || t == Game::ItemType::StrawberrySeed;
}

// 将“种子”物品映射为作物类型
inline CropType cropTypeFromSeed(Game::ItemType t) {
    switch (t) {
        case Game::ItemType::ParsnipSeed: return CropType::Parsnip;
        case Game::ItemType::BlueberrySeed: return CropType::Blueberry;
        case Game::ItemType::EggplantSeed: return CropType::Eggplant;
        case Game::ItemType::CornSeed: return CropType::Corn;
        case Game::ItemType::StrawberrySeed: return CropType::Strawberry;
        default: return CropType::Parsnip;
    }
}

}
