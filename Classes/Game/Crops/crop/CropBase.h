#pragma once

#include "cocos2d.h"
#include <array>
#include <vector>
#include "Game/Item.h"

namespace Game {

// 作物类型：用于索引作物静态定义（CropDefs）与行为实现（CropBase 派生类）。
enum class CropType { Parsnip, Blueberry, Eggplant, Corn, Strawberry };

// 作物运行时状态：由 CropSystem 作为唯一来源持有并推进。
struct Crop {
    int c = 0; // 网格列坐标（tile c）
    int r = 0; // 网格行坐标（tile r）
    CropType type = CropType::Parsnip; // 作物类型（决定静态定义/回生属性/物品映射）
    int stage = 0; // 当前生长阶段（0..maxStage；回生作物可能处于 maxStage“已采摘占位”）
    int progress = 0; // 当前阶段已推进的天数（浇水触发推进，达到 stageDays[stage] 后进入下一阶段）
    int maxStage = 0; // 最大阶段索引（通常等于 CropDefs::maxStage(type)，缓存以避免重复查询）
    bool wateredToday = false; // 当日是否被浇水（用于每日推进；推进后会被系统复位）
};

// 作物静态定义：用于贴图裁切与生长阶段天数等常量配置。
struct CropDef {
    int baseRow16 = 0; // 贴图底部行索引（以 16px 为单位，自底向上计数）
    int startCol = 0; // 贴图列起点（每个阶段通常占一列）
    std::vector<int> stageDays; // 各生长阶段持续天数（索引即 stage）
    std::array<bool, 4> seasons; // 四季可种植标记（0..3 对应 seasonIndex）
};

// 作物静态定义访问入口：
// - 以 CropType 为索引，返回纹理定位、阶段天数、季节与回生属性等常量。
// - 协作对象：各作物的 CropBase 派生实现（唯一提供 def_/regrow_ 等数据）。
class CropDefs {
public:
    // 获取某种作物的静态定义（引用长期有效）。
    static const CropDef& get(CropType t);
    // 获取每个阶段的持续天数数组（索引为 stage）。
    static const std::vector<int>& stageDays(CropType t);
    // 获取最大阶段索引（即 stageDays.size()-1）。
    static int maxStage(CropType t);
    // 获取贴图列起点（每个阶段通常占一列）。
    static int startCol(CropType t);
    // 获取贴图底部行索引（以 16px 为单位，自底向上计数）。
    static int baseRow16(CropType t);
    // 判断当前季节是否允许种植（seasonIndex: 0..3）。
    static bool isSeasonAllowed(CropType t, int seasonIndex);
    // 判断是否为回生作物（收获后可再次生长）。
    static bool isRegrow(CropType t);
};

// 将作物类型映射为“种子”物品类型（用于商店/播种）。
ItemType seedItemFor(CropType t);
// 将作物类型映射为“成熟产物”物品类型（用于收获/掉落）。
ItemType produceItemFor(CropType t);
// 判断物品是否为“种子”类（用于播种判定/喂鸡判定等）。
bool isSeed(ItemType t);
// 将“种子”物品类型映射回作物类型（用于从背包解析播种目标）。
CropType cropTypeFromSeed(ItemType t);

// 计算作物贴图下半块裁切矩形（16px 高）。
cocos2d::Rect cropRectBottomHalf(CropType t, int stage, float textureHeight);
// 计算作物贴图上半块裁切矩形（位于下半块之上 16px）。
cocos2d::Rect cropRectTopHalf(CropType t, int stage, float textureHeight);

// 作物行为/静态数据基类：
// - 提供作物的静态定义（def_）与回生属性（regrow_）等数据入口。
// - 仅承载“类型→定义/规则”的数据，不直接持有运行时实例列表。
// - 协作对象：CropSystem 通过 CropDefs 查询常量并推进作物；播种/收获通过 ItemType 映射函数协作。
class CropBase {
public:
    // 虚析构：允许通过基类引用销毁派生实例（当前派生为静态单例）。
    virtual ~CropBase() = default;

    // 返回该行为对象对应的作物类型。
    virtual CropType cropType() const = 0;

    CropDef def_{}; // 静态定义数据（贴图定位/阶段天数/季节等）
    ItemType seedItem_ = ItemType::ParsnipSeed; // 对应的“种子”物品类型
    ItemType produceItem_ = ItemType::Parsnip; // 对应的“成熟产物”物品类型
    bool regrow_ = false; // 是否回生（收获后可再次生长到可收获阶段）

    // 获取静态定义（纹理定位/阶段天数/季节）。
    const CropDef& def() const { return def_; }
    // 获取该作物对应的种子物品类型。
    ItemType seedItem() const { return seedItem_; }
    // 获取该作物成熟后的产物物品类型。
    ItemType produceItem() const { return produceItem_; }
    // 是否回生（收获后可再次长到可收获阶段）。
    bool regrow() const { return regrow_; }
};

}
