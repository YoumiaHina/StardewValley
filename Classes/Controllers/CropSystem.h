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
    // 每日推进：依据瓦片是否“浇水”，推进阶段并将浇水格恢复为“耕地”
    void advanceCropsDaily(IMapController* map) {
        for (auto& cp : _crops) {
            auto t = map ? map->getTile(cp.c, cp.r) : Game::TileType::Soil;
            bool watered = (t == Game::TileType::Watered);
            if (watered && cp.stage < cp.maxStage) {
                cp.progress += 1;
                const auto& days = Game::CropDefs::stageDays(cp.type);
                int need = (cp.stage >= 0 && cp.stage < static_cast<int>(days.size())) ? days[cp.stage] : 1;
                if (cp.progress >= need) { cp.stage += 1; cp.progress = 0; }
            }
            if (t == Game::TileType::Watered && map) { map->setTile(cp.c, cp.r, Game::TileType::Tilled); }
        }
        syncSave();
    }
    // 尝试收获（仅在成熟时移除作物；背包交互在更高层处理）
    void harvestCropAt(int c, int r) {
        int idx = findCropIndex(c, r);
        if (idx < 0) return;
        auto cp = _crops[idx];
        if (cp.stage >= cp.maxStage) {
            _crops.erase(_crops.begin() + idx);
            syncSave();
        }
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
        if (cp.stage < cp.maxStage) { cp.stage += 1; cp.progress = 0; syncSave(); }
    }

private:
    std::vector<Game::Crop> _crops;
    // 从 WorldState 读取/写回当前作物列表
    void syncLoad() { _crops = Game::globalState().farmCrops; }
    void syncSave() { Game::globalState().farmCrops = _crops; }
};

}
