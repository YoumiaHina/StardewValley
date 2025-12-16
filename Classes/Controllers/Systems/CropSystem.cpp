// 作物系统实现：
// - 管理农场上的作物列表与状态持久化
// - 每日推进依赖 CropDefs::stageDays；特殊回生由 CropBase 派生覆盖
// - 收获/加速等操作通过适配器委派给具体作物行为
#include "Controllers/Systems/CropSystem.h"
#include "Game/Crops/crop/CropBase.h"
#include <cstdlib>
// 行为实现由各作物派生在 *.cpp 中定义；此处仅声明获取入口
namespace Game {
    const CropBase& parsnipCropBehavior();
    const CropBase& blueberryCropBehavior();
    const CropBase& eggplantCropBehavior();
    const CropBase& cornCropBehavior();
    const CropBase& strawberryCropBehavior();
}

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

// 是否可收获：委派给行为实现
bool CropSystem::canHarvestAt(int c, int r) const {
    int idx = findCropIndex(c, r);
    if (idx < 0) return false;
    const auto& cp = _crops[idx];
    return behaviorFor(cp.type).canHarvest(cp);
}

// 收获是否产出：委派给行为实现
bool CropSystem::yieldsOnHarvestAt(int c, int r) const {
    int idx = findCropIndex(c, r);
    if (idx < 0) return false;
    const auto& cp = _crops[idx];
    return behaviorFor(cp.type).yieldsOnHarvest(cp);
}

// 每日推进：
// - 若行为 onDailyRegrow 已处理（例如成熟占位退回倒数阶段），则跳过常规增长
// - 否则在浇水条件下推进阶段内进度，并根据阶段天数决定是否升级阶段
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

        if (behaviorFor(cp.type).onDailyRegrow(cp)) {
        } else {
            if (watered && cp.stage < cp.maxStage) {
                cp.progress += 1;
                const auto& days = Game::CropDefs::stageDays(cp.type);
                int need = (cp.stage >= 0 && cp.stage < static_cast<int>(days.size())) ? days[cp.stage] : 1;
                if (cp.progress >= need) { cp.stage += 1; cp.progress = 0; }
            }
        }

        cp.wateredToday = false;

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

// 收获：由行为决定是否移除作物（remove=true）
void CropSystem::harvestCropAt(int c, int r) {
    int idx = findCropIndex(c, r);
    if (idx < 0) return;
    auto cp = _crops[idx];
    bool remove = false;
    behaviorFor(cp.type).onHarvest(_crops[idx], remove);
    if (remove) { _crops.erase(_crops.begin() + idx); }
    syncSave();
}

// 作弊：使所有作物瞬间成熟
void CropSystem::instantMatureAllCrops() {
    for (auto& cp : _crops) { cp.stage = cp.maxStage; cp.progress = 0; }
    syncSave();
}

// 加速一次：委派行为执行阶段推进或回退
void CropSystem::advanceCropOnceAt(int c, int r) {
    int idx = findCropIndex(c, r);
    if (idx < 0) return;
    auto& cp = _crops[idx];
    if (behaviorFor(cp.type).accelerate(cp)) { syncSave(); }
}

// 与全局状态同步（存/取）
void CropSystem::syncLoad() { _crops = Game::globalState().farmCrops; }
void CropSystem::syncSave() { Game::globalState().farmCrops = _crops; }

// 行为适配器：将 Game::CropBase 映射为本地 ICropBehavior 接口
const CropSystem::ICropBehavior& CropSystem::behaviorFor(Game::CropType t) const {
    struct Adapter : ICropBehavior {
        const Game::CropBase* impl;
        Adapter(const Game::CropBase* p) : impl(p) {}
        bool canAccelerate(const Game::Crop& cp) const override { return impl->canAccelerate(cp); }
        bool canHarvest(const Game::Crop& cp) const override { return impl->canHarvest(cp); }
        void onHarvest(Game::Crop& cp, bool& remove) const override { impl->onHarvest(cp, remove); }
        bool onDailyRegrow(Game::Crop& cp) const override { return impl->onDailyRegrow(cp); }
        bool accelerate(Game::Crop& cp) const override { return impl->accelerate(cp); }
        bool yieldsOnHarvest(const Game::Crop& cp) const override { return impl->yieldsOnHarvest(cp); }
    };
    static Adapter parsnip(&Game::parsnipCropBehavior());
    static Adapter blueberry(&Game::blueberryCropBehavior());
    static Adapter eggplant(&Game::eggplantCropBehavior());
    static Adapter corn(&Game::cornCropBehavior());
    static Adapter strawberry(&Game::strawberryCropBehavior());
    switch (t) {
        case Game::CropType::Blueberry: return blueberry;
        case Game::CropType::Eggplant: return eggplant;
        case Game::CropType::Corn: return corn;
        case Game::CropType::Strawberry: return strawberry;
        default: return parsnip;
    }
}

}
