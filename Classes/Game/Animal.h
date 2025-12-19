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
    AnimalType type = AnimalType::Chicken;
    cocos2d::Vec2 pos;
    cocos2d::Vec2 target;
    float speed = 0.0f;
    float wanderRadius = 0.0f;
    int ageDays = 0;
    bool isAdult = false;
    bool fedToday = false;
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
