// 玉米：倒数阶段可采摘一次，成熟阶段为“已采摘”占位；支持每日回生退回倒数阶段
#include <algorithm>
#include "Game/Crops/crop/CropBase.h"

namespace Game {

class CornCrop : public CropBase {
public:
    // 可被加速（作弊/道具）
    bool canAccelerate(const Game::Crop& cp) const override { return true; }
    // 倒数阶段可采摘；成熟阶段表示已采摘占位，可拔除
    bool canHarvest(const Game::Crop& cp) const override { return cp.stage == cp.maxStage - 1 || cp.stage >= cp.maxStage; }
    void onHarvest(Game::Crop& cp, bool& remove) const override {
        // 采摘：倒数阶段→成熟占位；成熟占位则拔除
        if (cp.stage == cp.maxStage - 1) { cp.stage = cp.maxStage; cp.progress = 0; remove = false; }
        else if (cp.stage >= cp.maxStage) { remove = true; }
    }
    bool onDailyRegrow(Game::Crop& cp) const override {
        // 每日回生：成熟占位退回到倒数阶段
        if (cp.stage >= cp.maxStage) { cp.stage = std::max(0, cp.maxStage - 1); cp.progress = 0; return true; }
        return false;
    }
    bool accelerate(Game::Crop& cp) const override {
        // 加速：若成熟占位则回退；否则正常推进
        if (cp.stage >= cp.maxStage) { cp.stage = std::max(0, cp.maxStage - 1); cp.progress = 0; return true; }
        if (cp.stage < cp.maxStage) { cp.stage += 1; cp.progress = 0; return true; }
        return false;
    }
    // 仅倒数阶段采摘产生成熟产物
    bool yieldsOnHarvest(const Game::Crop& cp) const override { return cp.stage == cp.maxStage - 1; }
};

const CropBase& cornCropBehavior() {
    static CornCrop inst;
    return inst;
}

}
