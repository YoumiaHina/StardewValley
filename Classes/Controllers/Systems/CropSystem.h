// 作物系统：
// - 持有并管理农场上的作物实例列表（位置/阶段/浇水等）
// - 负责每日推进与收获/加速等操作；具体行为转发给 CropBase 派生实现
// - 与地图控制器协作读写瓦片（浇水后回退为 Tilled）并刷新可视化
#pragma once

#include <vector>
#include "Game/Crops/crop/CropBase.h"
#include "Game/WorldState.h"
#include "Controllers/Map/IMapController.h"

namespace Controllers {

class CropSystem {
public:
    CropSystem();
    const std::vector<Game::Crop>& crops() const;
    std::vector<Game::Crop>& crops();
    int findCropIndex(int c, int r) const;
    void plantCrop(Game::CropType type, int c, int r);
    void markWateredAt(int c, int r);
    bool canHarvestAt(int c, int r) const;
    bool yieldsOnHarvestAt(int c, int r) const;
    void advanceCropsDaily(IMapController* map);
    void harvestCropAt(int c, int r);
    void instantMatureAllCrops();
    void advanceCropOnceAt(int c, int r);

private:
    std::vector<Game::Crop> _crops;
    void syncLoad();
    void syncSave();
};

}
