#pragma once

#include "Game/Animal.h"

namespace Game {

class AnimalBase {
public:
    virtual ~AnimalBase() = default;
    virtual AnimalType type() const = 0;
    virtual float moveSpeed() const = 0;
    virtual float wanderRadius() const = 0;
    virtual const char* texturePath() const = 0;
};

}

