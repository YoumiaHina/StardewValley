#include "Game/Crops/crop/CropBase.h"

namespace Game {

bool RegrowCropBase::canAccelerate(const Game::Crop& cp) const {
    return true;
}

bool RegrowCropBase::canHarvest(const Game::Crop& cp) const {
    return cp.stage == cp.maxStage - 1 || cp.stage >= cp.maxStage;
}

void RegrowCropBase::onHarvest(Game::Crop& cp, bool& remove) const {
    if (cp.stage == cp.maxStage - 1) {
        cp.stage = cp.maxStage;
        cp.progress = 0;
        remove = false;
    } else if (cp.stage >= cp.maxStage) {
        remove = true;
    }
}

bool RegrowCropBase::onDailyRegrow(Game::Crop& cp) const {
    if (cp.stage >= cp.maxStage) {
        cp.stage = std::max(0, cp.maxStage - 1);
        cp.progress = 0;
        return true;
    }
    return false;
}

bool RegrowCropBase::accelerate(Game::Crop& cp) const {
    if (cp.stage >= cp.maxStage) {
        cp.stage = std::max(0, cp.maxStage - 1);
        cp.progress = 0;
        return true;
    }
    if (cp.stage < cp.maxStage) {
        cp.stage += 1;
        cp.progress = 0;
        return true;
    }
    return false;
}

bool RegrowCropBase::yieldsOnHarvest(const Game::Crop& cp) const {
    return cp.stage == cp.maxStage - 1;
}

}

