#pragma once

#include "cocos2d.h"

namespace Game {

enum class AnimalType {
    Chicken,
    Cow,
    Sheep
};

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

inline long long animalPrice(AnimalType t) {
    switch (t) {
        case AnimalType::Chicken: return 800;
        case AnimalType::Cow: return 1500;
        case AnimalType::Sheep: return 2000;
    }
    return 0;
}

}
