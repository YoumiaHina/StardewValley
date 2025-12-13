#pragma once

#include "Game/Crop.h"

namespace Game {

class CropBase {
public:
    virtual ~CropBase() = default;
    virtual bool canAccelerate(const Game::Crop& cp) const = 0;
    virtual bool canHarvest(const Game::Crop& cp) const = 0;
    virtual void onHarvest(Game::Crop& cp, bool& remove) const = 0;
    virtual bool onDailyRegrow(Game::Crop& cp) const = 0;
    virtual bool accelerate(Game::Crop& cp) const = 0;
    virtual bool yieldsOnHarvest(const Game::Crop& cp) const = 0;
};

}
