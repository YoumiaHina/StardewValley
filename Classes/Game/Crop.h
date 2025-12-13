// 作物基础数据与物品映射：
// - 运行时实例 Crop：描述某格子的作物当前生长状态（不含行为）
// - 静态定义 CropDef/CropDefs：每种作物的纹理定位与阶段天数等常量
// - 物品映射：作物类型 ↔ 种子/成熟物品（用于背包与收获）
// 行为差异（可收获/回生/加速等）由 CropBase 及其派生类实现，见 Game/Crops/crop/*
#pragma once

#include <vector>
#include <array>
#include "Game/Item.h"

namespace Game {

// 作物类型枚举（用于索引静态定义与派生行为）
enum class CropType { Parsnip, Blueberry, Eggplant, Corn, Strawberry };

// 作物实例：记录所在格子、类型与当前生长状态（由 CropSystem 管理生命周期）
struct Crop {
    // 网格坐标（列c、行r）
    int c = 0;
    int r = 0;
    // 作物类型（用于查询静态定义与行为）
    CropType type = CropType::Parsnip;
    // 当前生长阶段与阶段内进度（单位：天）
    int stage = 0;
    int progress = 0;
    // 该作物的最大阶段（由 CropDefs::maxStage 初始化）
    int maxStage = 0;
    // 当日是否浇水（在每日推进后自动清零）
    bool wateredToday = false;
};

// 作物静态定义：纹理行(以16像素为单位，自底向上计数)、起始列与各阶段所需天数
// 说明：纹理坐标系以左下角为(0,0)，裁切见 Game/CropSprites.h
struct CropDef {
    int baseRow16 = 0;
    int startCol = 0;
    std::vector<int> stageDays;
    std::array<bool,4> seasons;
};

// 作物定义表：集中访问每种作物的静态属性（不含行为）
class CropDefs {
public:
    static const CropDef& get(CropType t);
    static const std::vector<int>& stageDays(CropType t);
    static int maxStage(CropType t);
    static int startCol(CropType t);
    static int baseRow16(CropType t);
    static bool isSeasonAllowed(CropType t, int seasonIndex);
};

// 根据作物类型返回对应的“种子”物品类型（用于商店/播种）
Game::ItemType seedItemFor(CropType t);

// 根据作物类型返回对应的“收获产物”物品类型（用于收获入背包/掉落）
Game::ItemType produceItemFor(CropType t);

// 是否为“种子”类物品（用于播种判定）
bool isSeed(Game::ItemType t);

// 将“种子”物品映射为作物类型（用于从物品栏解析播种目标）
CropType cropTypeFromSeed(Game::ItemType t);

}
