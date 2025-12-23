#pragma once

#include "cocos2d.h"

namespace Game {

// 动物类型：用于索引静态行为（AnimalBase 派生）与系统逻辑（AnimalSystem）。
enum class AnimalType {
    Chicken,
    Cow,
    Sheep
};

// 动物运行时状态：由 AnimalSystem 作为唯一来源持有并推进。
struct Animal {
    AnimalType type = AnimalType::Chicken; // 动物类型（决定静态行为/产物规则）
    cocos2d::Vec2 pos; // 当前世界坐标位置
    cocos2d::Vec2 target; // 当前游走目标点（世界坐标）
    float speed = 0.0f; // 移动速度（像素/秒或与地图坐标系一致）
    float wanderRadius = 0.0f; // 游走半径（以 tile 为单位，系统侧统一换算）
    int ageDays = 0; // 已累计“有效喂食天数”（达到 matureDays 后成年）
    bool isAdult = false; // 是否成年（成年且当日喂食后可产出）
    bool fedToday = false; // 当日是否已喂食（每日推进后由系统复位）
};

// 动物购买价格：用于商店购买时扣费。
inline long long animalPrice(AnimalType t) {
    switch (t) {
        case AnimalType::Chicken: return 800;
        case AnimalType::Cow: return 1500;
        case AnimalType::Sheep: return 2000;
    }
    return 0;
}

}
