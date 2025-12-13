#include <algorithm>
#include "Game/Crops/crop/CropBase.h"

namespace Game {

class CornCrop : public CropBase {
public:
    bool canAccelerate(const Game::Crop& cp) const override { return true; }
    bool canHarvest(const Game::Crop& cp) const override { return cp.stage == cp.maxStage - 1 || cp.stage >= cp.maxStage; }
    void onHarvest(Game::Crop& cp, bool& remove) const override {
        if (cp.stage == cp.maxStage - 1) { cp.stage = cp.maxStage; cp.progress = 0; remove = false; }
        else if (cp.stage >= cp.maxStage) { remove = true; }
    }
    bool onDailyRegrow(Game::Crop& cp) const override {
        if (cp.stage >= cp.maxStage) { cp.stage = std::max(0, cp.maxStage - 1); cp.progress = 0; return true; }
        return false;
    }
    bool accelerate(Game::Crop& cp) const override {
        if (cp.stage >= cp.maxStage) { cp.stage = std::max(0, cp.maxStage - 1); cp.progress = 0; return true; }
        if (cp.stage < cp.maxStage) { cp.stage += 1; cp.progress = 0; return true; }
        return false;
    }
    bool yieldsOnHarvest(const Game::Crop& cp) const override { return cp.stage == cp.maxStage - 1; }
};

const CropBase& cornCropBehavior() {
    static CornCrop inst;
    return inst;
}

}
