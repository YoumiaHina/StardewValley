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
    static const CropDef& get(CropType t);
    static const std::vector<int>& stageDays(CropType t);
    static int maxStage(CropType t);
    static int startCol(CropType t);
    static int baseRow16(CropType t);
    static bool isSeasonAllowed(CropType t, int seasonIndex);
};

// 根据作物类型返回对应的“种子”物品类型
Game::ItemType seedItemFor(CropType t);

// 根据作物类型返回对应的“收获产物”物品类型
Game::ItemType produceItemFor(CropType t);

// 是否为“种子”类物品（用于播种判定）
bool isSeed(Game::ItemType t);

// 将“种子”物品映射为作物类型
CropType cropTypeFromSeed(Game::ItemType t);

}
