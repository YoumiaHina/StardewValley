// 萝卜：一次性成熟，可收获即移除，不回生
#include <algorithm>
#include "Game/Crops/crop/CropBase.h"

namespace Game {

class ParsnipCrop : public CropBase {
public:
    // 可被加速（作弊/道具）
    bool canAccelerate(const Game::Crop& cp) const override { return true; }
    // 仅在成熟阶段可收获
    bool canHarvest(const Game::Crop& cp) const override { return cp.stage >= cp.maxStage; }
    // 收获后直接移除
    void onHarvest(Game::Crop& cp, bool& remove) const override { remove = (cp.stage >= cp.maxStage); }
    // 不回生
    bool onDailyRegrow(Game::Crop& cp) const override { return false; }
    bool accelerate(Game::Crop& cp) const override {
        // 阶段推进一格，重置阶段内进度
        if (cp.stage < cp.maxStage) { cp.stage += 1; cp.progress = 0; return true; }
        return false;
    }
    // 成熟阶段收获产出
    bool yieldsOnHarvest(const Game::Crop& cp) const override { return cp.stage >= cp.maxStage; }
};

const CropBase& parsnipCropBehavior() {
    static ParsnipCrop inst;
    return inst;
}

}
