// 作物系统实现：
// - 管理农场上的作物列表与状态持久化
// - 每日推进依赖 CropDefs::stageDays；回生/收获/加速由系统统一分支处理
#include "Controllers/Systems/CropSystem.h"
#include <algorithm>
#include <cstdlib>

namespace Controllers {

// 初始化：从全局状态加载农场作物列表
CropSystem::CropSystem() { syncLoad(); }
const std::vector<Game::Crop>& CropSystem::crops() const { return _crops; }
std::vector<Game::Crop>& CropSystem::crops() { return _crops; }

// 根据网格坐标查找作物索引；未找到返回 -1
int CropSystem::findCropIndex(int c, int r) const {
    for (int i = 0; i < static_cast<int>(_crops.size()); ++i) {
        if (_crops[i].c == c && _crops[i].r == r) return i;
    }
    return -1;
}

// 种植：创建作物实例并初始化最大阶段
void CropSystem::plantCrop(Game::CropType type, int c, int r) {
    Game::Crop cp; cp.c = c; cp.r = r; cp.type = type; cp.stage = 0; cp.progress = 0; cp.maxStage = Game::CropDefs::maxStage(type);
    _crops.push_back(cp);
    syncSave();
}

// 标记浇水：用于当日推进时计算是否增长
void CropSystem::markWateredAt(int c, int r) {
    int idx = findCropIndex(c, r);
    if (idx < 0) return;
    _crops[idx].wateredToday = true;
    syncSave();
}

// 是否可收获
bool CropSystem::canHarvestAt(int c, int r) const {
    int idx = findCropIndex(c, r);
    if (idx < 0) return false;
    const auto& cp = _crops[idx];
    if (Game::CropDefs::isRegrow(cp.type)) {
        int penultimate = std::max(0, cp.maxStage - 1);
        return cp.stage == penultimate || cp.stage >= cp.maxStage;
    }
    return cp.stage >= cp.maxStage;
}

// 收获是否产出
bool CropSystem::yieldsOnHarvestAt(int c, int r) const {
    int idx = findCropIndex(c, r);
    if (idx < 0) return false;
    const auto& cp = _crops[idx];
    if (Game::CropDefs::isRegrow(cp.type)) {
        int penultimate = std::max(0, cp.maxStage - 1);
        return cp.stage == penultimate;
    }
    return cp.stage >= cp.maxStage;
}

// 每日推进：
// - 普通作物：浇水则按阶段天数推进到 maxStage
// - 回生作物：浇水则推进到倒数第二阶段；收获后处于 maxStage 占位，再浇水从 maxStage 长回倒数第二阶段
void CropSystem::advanceCropsDaily(IMapController* map) {
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

        bool outOfSeason = !Game::CropDefs::isSeasonAllowed(cp.type, ws.seasonIndex);
        bool died = outOfSeason;
        if (!died && !watered) {
            double roll = static_cast<double>(std::rand()) / (static_cast<double>(RAND_MAX) + 1.0);
            if (roll < 0.15) {
                died = true;
            }
        }

        if (died) {
            cp.wateredToday = false;
            continue;
        }

        bool regrow = Game::CropDefs::isRegrow(cp.type);
        int penultimate = std::max(0, cp.maxStage - 1);
        int growMaxStage = regrow ? penultimate : cp.maxStage;

        if (regrow && cp.stage >= cp.maxStage) {
            if (watered) {
                cp.progress += 1;
                const auto& days = Game::CropDefs::stageDays(cp.type);
                int idx = cp.maxStage;
                int need = (idx >= 0 && idx < static_cast<int>(days.size())) ? std::max(1, days[idx]) : 1;
                if (cp.progress >= need) {
                    cp.stage = penultimate;
                    cp.progress = 0;
                }
            }
        } else if (watered && cp.stage < growMaxStage) {
            cp.progress += 1;
            const auto& days = Game::CropDefs::stageDays(cp.type);
            int need = (cp.stage >= 0 && cp.stage < static_cast<int>(days.size())) ? std::max(1, days[cp.stage]) : 1;
            if (cp.progress >= need) { cp.stage += 1; cp.progress = 0; }
        }

        cp.wateredToday = false;
        kept.push_back(cp);
    }
    _crops.swap(kept);
    syncSave();

    if (map && map->isFarm()) {
        if (cols > 0 && rows > 0) {
            for (int r = 0; r < rows; ++r) {
                for (int c = 0; c < cols; ++c) {
                    if (!map->inBounds(c, r)) continue;
                    auto t = map->getTile(c, r);
                    if (t == Game::TileType::Watered) {
                        map->setTile(c, r, Game::TileType::Tilled);
                    }
                }
            }
        }
    } else if (canCheckTiles) {
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                int idx = r * cols + c;
                if (idx >= 0 && idx < static_cast<int>(ws.farmTiles.size())) {
                    if (ws.farmTiles[static_cast<std::size_t>(idx)] == Game::TileType::Watered) {
                        ws.farmTiles[static_cast<std::size_t>(idx)] = Game::TileType::Tilled;
                    }
                }
            }
        }
    }
}

// 收获
void CropSystem::harvestCropAt(int c, int r) {
    int idx = findCropIndex(c, r);
    if (idx < 0) return;
    auto& cp = _crops[idx];
    if (Game::CropDefs::isRegrow(cp.type)) {
        int penultimate = std::max(0, cp.maxStage - 1);
        if (cp.stage == penultimate) {
            cp.stage = cp.maxStage;
            cp.progress = 0;
        } else if (cp.stage >= cp.maxStage) {
            _crops.erase(_crops.begin() + idx);
        }
    } else {
        if (cp.stage >= cp.maxStage) { _crops.erase(_crops.begin() + idx); }
    }
    syncSave();
}

// 作弊：使所有作物瞬间成熟
void CropSystem::instantMatureAllCrops() {
    for (auto& cp : _crops) {
        if (Game::CropDefs::isRegrow(cp.type)) {
            cp.stage = std::max(0, cp.maxStage - 1);
        } else {
            cp.stage = cp.maxStage;
        }
        cp.progress = 0;
    }
    syncSave();
}

// 加速一次
void CropSystem::advanceCropOnceAt(int c, int r) {
    int idx = findCropIndex(c, r);
    if (idx < 0) return;
    auto& cp = _crops[idx];
    bool regrow = Game::CropDefs::isRegrow(cp.type);
    int penultimate = std::max(0, cp.maxStage - 1);
    int growMaxStage = regrow ? penultimate : cp.maxStage;

    bool changed = false;
    if (regrow && cp.stage >= cp.maxStage) {
        cp.stage = penultimate;
        cp.progress = 0;
        changed = true;
    } else if (cp.stage < growMaxStage) {
        cp.stage += 1;
        cp.progress = 0;
        changed = true;
    }

    if (changed) { syncSave(); }
}

// 与全局状态同步（存/取）
void CropSystem::syncLoad() { _crops = Game::globalState().farmCrops; }
void CropSystem::syncSave() { Game::globalState().farmCrops = _crops; }

}
