/**
 * FarmMapController：农场地图控制器。
 * - 职责：加载并管理农场 TMX、维护农场瓦片类型、坐标换算与交互光标、门/区域检测、以及掉落/箱子/作物可视刷新入口。
 * - 职责边界：不在此处实现工具命中/掉落规则等业务细节；树/石等环境实体状态由对应 System 作为唯一来源维护。
 * - 协作对象：通过 IMapController 接口与 PlayerController/ToolSystem/DropSystem/ChestController/FurnaceController 以及环境 System 协作。
 */
#pragma once

#include "cocos2d.h"
#include <vector>
#include <utility>
#include <memory>
#include <unordered_map>
#include "Controllers/Map/IMapController.h"
#include "Game/GameConfig.h"
#include "Game/Map/FarmMap.h"
#include "Game/Chest.h"
#include "Game/WorldState.h"
#include "Controllers/Environment/TreeSystem.h"
#include "Controllers/Environment/RockSystem.h"
#include "Controllers/Environment/WeedSystem.h"
#include "Controllers/Environment/EnvironmentObstacleSystemBase.h"
#include "Controllers/Interact/TileSelector.h"
#include "Controllers/Systems/ChestController.h"
#include "Controllers/Systems/FurnaceController.h"
#include "Controllers/Systems/DropSystem.h"

namespace Controllers {

class FarmMapController : public Controllers::IMapController {
public:

    // 构造：绑定 FarmMap 与世界节点。
    FarmMapController(Game::FarmMap* map, cocos2d::Node* worldNode);

    // 初始化：创建 TMX、挂接渲染节点与系统，并同步存档状态。
    void init();

    // IMapController overrides
    // 将玩家在地图局部坐标转换为世界节点坐标系位置。
    cocos2d::Vec2 getPlayerPosition(const cocos2d::Vec2& playerMapLocalPos) const override;
    // 获取地图内容尺寸（像素）。
    cocos2d::Size getContentSize() const override;
    // 获取地图在世界节点中的原点偏移（用于相机计算）。
    cocos2d::Vec2 getOrigin() const override { return _mapOrigin; }
    // 将玩家下一帧位置夹紧到地图边界/碰撞规则允许范围内。
    cocos2d::Vec2 clampPosition(const cocos2d::Vec2& current,
                                 const cocos2d::Vec2& next,
                                 float radius) const override;
    // 检测指定位置是否与地图/障碍发生碰撞。
    bool collides(const cocos2d::Vec2& pos, float radius) const override;

    // 是否靠近农场门（农场→房间）。
    bool isNearDoor(const cocos2d::Vec2& playerWorldPos) const override;
    // 是否靠近矿洞门（农场→矿洞）。
    bool isNearMineDoor(const cocos2d::Vec2& playerWorldPos) const override;
    // 是否靠近沙滩门（农场→沙滩）。
    bool isNearBeachDoor(const cocos2d::Vec2& playerWorldPos) const override;
    // 是否靠近城镇门（农场→城镇）。
    bool isNearTownDoor(const cocos2d::Vec2& playerWorldPos) const override;
    // 是否靠近箱子交互区域。
    bool isNearChest(const cocos2d::Vec2& playerWorldPos) const override;

    // 获取瓦片尺寸（像素）。
    float tileSize() const override { return static_cast<float>(GameConfig::TILE_SIZE); }
    // 检查瓦片索引是否在边界内。
    bool inBounds(int c, int r) const override;
    // 计算玩家面向/点击目标瓦片索引。
    std::pair<int,int> targetTile(const cocos2d::Vec2& playerPos,
                                  const cocos2d::Vec2& lastDir) const override;
    // 更新交互光标（扇形选择）。
    void updateCursor(const cocos2d::Vec2& playerPos,
                      const cocos2d::Vec2& lastDir) override;

    // 获取瓦片类型。
    Game::TileType getTile(int c, int r) const override;
    // 设置瓦片类型（用于耕地/障碍生成等）。
    void setTile(int c, int r, Game::TileType t) override;
    // 瓦片索引转世界坐标（瓦片中心点）。
    cocos2d::Vec2 tileToWorld(int c, int r) const override;
    // 世界坐标转瓦片索引。
    void worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const override;

    // 获取指定障碍系统实例（树/石等）。
    EnvironmentObstacleSystemBase* obstacleSystem(ObstacleKind kind) override;
    // 获取指定障碍系统实例（只读）。
    const EnvironmentObstacleSystemBase* obstacleSystem(ObstacleKind kind) const override;

    // 刷新地图可视（瓦片覆层/箱子等）。
    void refreshMapVisuals() override;
    // 刷新作物可视（作物精灵与顶层精灵）。
    void refreshCropsVisuals() override;
    // 刷新掉落物可视。
    void refreshDropsVisuals() override;
    // 在指定瓦片生成掉落物。
    void spawnDropAt(int c, int r, int itemType, int qty) override;

    // 获取箱子列表（只读）。
    const std::vector<Game::Chest>& chests() const override;
    // 获取箱子列表（可写）。
    std::vector<Game::Chest>& chests() override;

    // 获取熔炉控制器指针（由农场控制器持有/挂接）。
    Controllers::FurnaceController* furnaceController() const { return _furnaceController; }

    // 获取农场 TMX 封装对象。
    Game::FarmMap* tmx() const { return _farmMap; }
    // 获取世界根节点。
    cocos2d::Node* worldNode() const { return _worldNode; }
    // 将角色节点挂到正确父节点与层级。
    void addActorToMap(cocos2d::Node* node, int zOrder) override;
    void addActorToOverlay(cocos2d::Node* node, int zOrder) override;
    // 获取从农场进入矿洞的出生点。
    cocos2d::Vec2 farmMineDoorSpawnPos() const override;
    // 获取从农场进入房间的出生点。
    cocos2d::Vec2 farmRoomDoorSpawnPos() const override;
    // 获取从农场进入沙滩的出生点。
    cocos2d::Vec2 farmBeachDoorSpawnPos() const override;
    // 获取从农场进入城镇的出生点。
    cocos2d::Vec2 farmTownDoorSpawnPos() const override;

    // 是否靠近湖泊/水域（用于钓鱼等）。
    bool isNearLake(const cocos2d::Vec2& playerWorldPos, float radius) const override;
    // 将角色渲染层级与环境节点做排序（遮挡关系）。
    void sortActorWithEnvironment(cocos2d::Node* actor) override;
    // 是否为农场地图（供外部做场景分支）。
    bool isFarm() const override { return true; }
    // 记录最近一次点击的世界坐标（用于三格选择）。
    void setLastClickWorldPos(const cocos2d::Vec2& p) override;
    // 清除最近一次点击记录。
    void clearLastClickWorldPos() override { _hasLastClick = false; }

    // 收集玩家附近掉落物到背包。
    void collectDropsNear(const cocos2d::Vec2& playerWorldPos, Game::Inventory* inv) override;

    // 是否支持天气系统。
    bool supportsWeather() const override { return true; }
    // 将所有可种植瓦片设为已浇水。
    void setAllPlantableTilesWatered() override;

private:
    cocos2d::Node* _worldNode = nullptr;
    cocos2d::Node* _mapNode = nullptr;
    Game::FarmMap* _farmMap = nullptr;
    int _cols = GameConfig::MAP_COLS;
    int _rows = GameConfig::MAP_ROWS;
    std::vector<Game::TileType> _tiles;
    cocos2d::DrawNode* _cursor = nullptr;
    cocos2d::Vec2 _mapOrigin;

    // Prompts/regions
    cocos2d::Rect _farmDoorRect;

    // Drops / Chests / Crops
    Controllers::DropSystem _dropSystem;
    Controllers::ChestController* _chestController = nullptr;
    Controllers::FurnaceController* _furnaceController = nullptr;
    cocos2d::DrawNode* _cropsDraw = nullptr;
    cocos2d::Node* _cropsRootBack = nullptr;
    cocos2d::Node* _cropsRootFront = nullptr;
    std::unordered_map<long long, cocos2d::Sprite*> _cropSprites;
    std::unordered_map<long long, cocos2d::Sprite*> _cropSpritesTop;

    // Tilled tile overlay sprites
    cocos2d::Node* _tileRoot = nullptr;
    std::unordered_map<long long, cocos2d::Sprite*> _tileSprites;
    std::unordered_map<long long, cocos2d::Sprite*> _waterSprites;
    cocos2d::Node* _actorsRoot = nullptr;
    Controllers::EnvironmentObstacleSystemBase* _treeSystem = nullptr;
    Controllers::EnvironmentObstacleSystemBase* _rockSystem = nullptr;
    Controllers::EnvironmentObstacleSystemBase* _weedSystem = nullptr;
    cocos2d::Vec2 _lastClickWorldPos = cocos2d::Vec2::ZERO;
    bool _hasLastClick = false;

    // 应用静态不可耕作区域遮罩（建筑/道路等）。
    void applyStaticNotSoilMask();
};

}
// namespace Controllers
