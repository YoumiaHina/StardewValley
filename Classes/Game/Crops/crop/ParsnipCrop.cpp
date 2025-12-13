#include <algorithm>
#include "Game/Crops/crop/CropBase.h"

namespace Game {

class ParsnipCrop : public CropBase {
public:
    bool canAccelerate(const Game::Crop& cp) const override { return true; }
    bool canHarvest(const Game::Crop& cp) const override { return cp.stage >= cp.maxStage; }
    void onHarvest(Game::Crop& cp, bool& remove) const override { remove = (cp.stage >= cp.maxStage); }
    bool onDailyRegrow(Game::Crop& cp) const override { return false; }
    bool accelerate(Game::Crop& cp) const override {
        if (cp.stage < cp.maxStage) { cp.stage += 1; cp.progress = 0; return true; }
        return false;
    }
    bool yieldsOnHarvest(const Game::Crop& cp) const override { return cp.stage >= cp.maxStage; }
};

const CropBase& parsnipCropBehavior() {
    static ParsnipCrop inst;
    return inst;
}

}
