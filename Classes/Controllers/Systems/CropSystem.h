#pragma once

#include <vector>
#include "Game/Crop.h"
#include "Game/WorldState.h"
#include "Controllers/IMapController.h"

namespace Controllers {

// 作物系统：集中管理作物状态（播种/推进/收获），并与 WorldState 同步
class CropSystem {
public:
    // 构造时从 WorldState 恢复作物列表
    CropSystem() { syncLoad(); }
    // 获取/修改当前所有作物实例（持久化于 WorldState::farmCrops）
    const std::vector<Game::Crop>& crops() const { return _crops; }
    std::vector<Game::Crop>& crops() { return _crops; }
    // 查找某格子的作物索引（不存在返回 -1）
    int findCropIndex(int c, int r) const {
        for (int i = 0; i < static_cast<int>(_crops.size()); ++i) {
            if (_crops[i].c == c && _crops[i].r == r) return i;
        }
        return -1;
    }
    // 在指定格子播种为给定作物类型（初始阶段）
    void plantCrop(Game::CropType type, int c, int r) {
        Game::Crop cp; cp.c = c; cp.r = r; cp.type = type; cp.stage = 0; cp.progress = 0; cp.maxStage = Game::CropDefs::maxStage(type);
        _crops.push_back(cp);
        syncSave();
    }
    void markWateredAt(int c, int r) {
        int idx = findCropIndex(c, r);
        if (idx < 0) return;
        _crops[idx].wateredToday = true;
        syncSave();
    }
    bool canHarvestAt(int c, int r) const {
        int idx = findCropIndex(c, r);
        if (idx < 0) return false;
        const auto& cp = _crops[idx];
        return behaviorFor(cp.type).canHarvest(cp);
    }
    bool yieldsOnHarvestAt(int c, int r) const {
        int idx = findCropIndex(c, r);
        if (idx < 0) return false;
        const auto& cp = _crops[idx];
        // 默认：最后阶段产出；蓝莓/特殊作物可覆盖
        return behaviorFor(cp.type).yieldsOnHarvest(cp);
    }
    // 每日推进：依据瓦片是否“浇水”，推进阶段并将浇水格恢复为“耕地”
    void advanceCropsDaily(IMapController* map) {
        auto &ws = Game::globalState();
        int cols = ws.farmCols;
        int rows = ws.farmRows;
        bool canCheckTiles = (cols > 0 && rows > 0 && ws.farmTiles.size() == static_cast<size_t>(cols * rows));

        std::vector<Game::Crop> kept;
        kept.reserve(_crops.size());
        for (auto& cp : _crops) {
            Game::TileType t = Game::TileType::Soil;
            int tileIdx = -1;
            if (canCheckTiles && cp.c >= 0 && cp.c < cols && cp.r >= 0 && cp.r < rows) {
                tileIdx = cp.r * cols + cp.c;
                t = ws.farmTiles[tileIdx];
            } else if (map && map->isFarm()) {
                t = map->getTile(cp.c, cp.r);
            }

            bool watered = cp.wateredToday || (t == Game::TileType::Watered);
            
            bool keep = true;
            if (behaviorFor(cp.type).onDailyRegrow(cp)) {
                // regrow handled
            } else {
                if (watered && cp.stage < cp.maxStage) {
                    cp.progress += 1;
                    const auto& days = Game::CropDefs::stageDays(cp.type);
                    int need = (cp.stage >= 0 && cp.stage < static_cast<int>(days.size())) ? days[cp.stage] : 1;
                    if (cp.progress >= need) { cp.stage += 1; cp.progress = 0; }
                }
            }
            
            cp.wateredToday = false;
            
            // Reset watered tile to tilled
            if (t == Game::TileType::Watered) {
                if (map && map->isFarm()) {
                    map->setTile(cp.c, cp.r, Game::TileType::Tilled);
                } else if (tileIdx >= 0) {
                    ws.farmTiles[tileIdx] = Game::TileType::Tilled;
                }
            }
            kept.push_back(cp);
        }
        _crops.swap(kept);
        syncSave();
    }
    // 尝试收获（仅在成熟时移除作物；背包交互在更高层处理）
    void harvestCropAt(int c, int r) {
        int idx = findCropIndex(c, r);
        if (idx < 0) return;
        auto cp = _crops[idx];
        bool remove = false;
        behaviorFor(cp.type).onHarvest(_crops[idx], remove);
        if (remove) { _crops.erase(_crops.begin() + idx); }
        syncSave();
    }
    // 使所有作物瞬间成熟
    void instantMatureAllCrops() {
        for (auto& cp : _crops) { cp.stage = cp.maxStage; cp.progress = 0; }
        syncSave();
    }
    // 使目标格的作物提升一个阶段
    void advanceCropOnceAt(int c, int r) {
        int idx = findCropIndex(c, r);
        if (idx < 0) return;
        auto& cp = _crops[idx];
        if (behaviorFor(cp.type).accelerate(cp)) { syncSave(); }
    }

private:
    std::vector<Game::Crop> _crops;
    // 从 WorldState 读取/写回当前作物列表
    void syncLoad() { _crops = Game::globalState().farmCrops; }
    void syncSave() { Game::globalState().farmCrops = _crops; }
    struct ICropBehavior {
        virtual bool canAccelerate(const Game::Crop& cp) const = 0;
        virtual bool canHarvest(const Game::Crop& cp) const = 0;
        virtual void onHarvest(Game::Crop& cp, bool& remove) const = 0;
        virtual bool onDailyRegrow(Game::Crop& cp) const = 0;
        virtual bool accelerate(Game::Crop& cp) const = 0;
        virtual bool yieldsOnHarvest(const Game::Crop& cp) const = 0;
        virtual ~ICropBehavior() = default;
    };
    struct DefaultBehavior : ICropBehavior {
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
    struct BlueberryBehavior : ICropBehavior {
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
    struct CornBehavior : ICropBehavior {
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
    struct StrawberryBehavior : ICropBehavior {
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
    struct EggplantBehavior : ICropBehavior {
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
    const ICropBehavior& behaviorFor(Game::CropType t) const {
        static DefaultBehavior def;
        static BlueberryBehavior bb;
        static EggplantBehavior eb;
        static CornBehavior cb;
        static StrawberryBehavior sb;
        switch (t) {
            case Game::CropType::Blueberry: return bb;
            case Game::CropType::Eggplant: return eb;
            case Game::CropType::Corn: return cb;
            case Game::CropType::Strawberry: return sb;
            default: return def;
        }
    }
};
}
