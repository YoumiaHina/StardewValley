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

// 作物系统（唯一来源）：
// - 负责农场作物的运行时状态列表（位置/阶段/浇水等）与持久化同步。
// - 通过 Game::CropDefs 查询静态定义（阶段天数/季节/回生）以推进作物。
// - 协作对象：IMapController 提供瓦片/坐标与掉落刷新；UI/工具通过该系统查询与操作作物。
class CropSystem {
public:
    // 锄头收获：若可收获则计算产物数量（含技能树加成）并执行收获；返回是否发生收获动作。
    bool harvestByHoeAt(int c, int r, int toolLevel, Game::ItemType& outProduce, int& outQty, bool& outYields);
    // 构造并从 WorldState 同步加载作物列表。
    CropSystem();
    // 只读访问作物列表（运行时状态由系统维护）。
    const std::vector<Game::Crop>& crops() const;
    // 可写访问作物列表（仅供系统内部/受控调用使用）。
    std::vector<Game::Crop>& crops();
    // 查找某格子上的作物索引；未找到返回 -1。
    int findCropIndex(int c, int r) const;
    // 在指定格子种植作物（初始化 stage/progress/maxStage）。
    void plantCrop(Game::CropType type, int c, int r);
    // 标记指定格子作物当日已浇水。
    void markWateredAt(int c, int r);
    // 判断指定格子作物是否达到可收获状态。
    bool canHarvestAt(int c, int r) const;
    // 判断指定格子作物收获时是否产出（回生作物在占位成熟阶段可能只拔除）。
    bool yieldsOnHarvestAt(int c, int r) const;
    // 每日推进：处理浇水/枯死/阶段增长，并回退 Watered 瓦片为 Tilled。
    void advanceCropsDaily(IMapController* map);
    // 收获指定格子作物：可能拔除或将回生作物转为占位成熟阶段。
    void harvestCropAt(int c, int r);
    // 作弊接口：将所有作物瞬间推进到可收获阶段。
    void instantMatureAllCrops();
    // 加速接口：将指定格子作物推进一次（不消耗浇水判定）。
    void advanceCropOnceAt(int c, int r);

private:
    // 作物运行时状态列表（std::vector：动态数组容器，会按需扩容）。
    std::vector<Game::Crop> _crops;
    // 从 WorldState 读取作物列表到本地缓存。
    void syncLoad();
    // 将本地作物列表写回 WorldState。
    void syncSave();
};

}
