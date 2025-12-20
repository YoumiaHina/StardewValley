/**
 * TownMapController：城镇地图控制器。
 * - 职责：提供城镇场景的尺寸/原点、碰撞夹紧、门与交互区域检测、瓦片坐标换算与光标选择、掉落与箱子表现刷新。
 * - 职责边界：不在此处实现耕地/作物等农场业务逻辑，仅对外暴露地图级接口。
 * - 协作对象：通过 IMapController 接口与 PlayerController/ToolSystem/UIController/DropSystem 等模块协作。
 */
#pragma once

#include "Controllers/Map/IMapController.h"
#include "Game/Map/TownMap.h"
#include "Controllers/Systems/DropSystem.h"

namespace Controllers {

class FurnaceController;

class TownMapController : public IMapController {
public:
    // 构造：绑定 TownMap 与世界节点，并初始化地图原点/瓦片网格/交互系统。
    explicit TownMapController(Game::TownMap* map, cocos2d::Node* worldNode);

    // 将玩家在地图局部坐标转换为世界节点坐标系位置。
    cocos2d::Vec2 getPlayerPosition(const cocos2d::Vec2& playerMapLocalPos) const override;

    // 将角色节点挂到正确父节点与层级。
    void addActorToMap(cocos2d::Node* node, int zOrder) override;

    // 获取地图内容尺寸（像素）。
    cocos2d::Size getContentSize() const override { return _map ? _map->getContentSize() : cocos2d::Size::ZERO; }
    // 获取地图在世界节点中的原点偏移（用于相机计算）。
    cocos2d::Vec2 getOrigin() const override { return _origin; }

    // 是否支持天气系统。
    bool supportsWeather() const override { return true; }
    // 将所有可种植瓦片设为已浇水（城镇无种植逻辑，保持空实现）。
    void setAllPlantableTilesWatered() override {}

    // 获取瓦片尺寸（像素）。
    float tileSize() const override { return _map ? _map->getTileSize().width : 0.0f; }
    // 世界坐标转换为瓦片索引。
    void worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const override;
    // 瓦片索引转换为世界坐标中心点。
    cocos2d::Vec2 tileToWorld(int c, int r) const override;
    // 判断瓦片索引是否在地图范围内。
    bool inBounds(int c, int r) const override;

    // 依据碰撞与边界规则夹紧移动位置。
    cocos2d::Vec2 clampPosition(const cocos2d::Vec2& current,
                                const cocos2d::Vec2& next,
                                float radius) const override;
    // 进行点位碰撞检测（含动态交互物）。
    bool collides(const cocos2d::Vec2& p, float radius) const override;

    // 是否靠近通往农场的门。
    bool isNearDoor(const cocos2d::Vec2& p) const override;
    // 是否靠近工具升级交互区域。
    bool isNearToolUpdateArea(const cocos2d::Vec2& p) const;
    // 是否靠近矿洞门（城镇不支持）。
    bool isNearMineDoor(const cocos2d::Vec2&) const override { return false; }
    // 是否靠近任意箱子。
    bool isNearChest(const cocos2d::Vec2& p) const override { return Game::isNearAnyChest(p, _chests); }
    // 是否靠近水域（城镇不支持，返回 false）。
    bool isNearLake(const cocos2d::Vec2&, float) const override { return false; }

    // 获取指定瓦片类型（城镇默认全 NotSoil）。
    Game::TileType getTile(int c, int r) const override;
    // 设置指定瓦片类型（城镇固定 NotSoil，忽略输入类型）。
    void setTile(int c, int r, Game::TileType t) override;

    // 依据玩家位置与朝向选择目标瓦片。
    std::pair<int,int> targetTile(const cocos2d::Vec2& p, const cocos2d::Vec2& lastDir) const override;
    // 更新光标显示（扇形选格）。
    void updateCursor(const cocos2d::Vec2& playerPos,
                      const cocos2d::Vec2& lastDir) override;

    // 刷新地图可视（主要为箱子等调试/占位显示）。
    void refreshMapVisuals() override;
    // 刷新作物可视（城镇无作物）。
    void refreshCropsVisuals() override {}
    // 刷新掉落物可视。
    void refreshDropsVisuals() override;
    // 在指定瓦片生成掉落物。
    void spawnDropAt(int c, int r, int itemType, int qty) override;
    // 收集玩家附近掉落物到背包。
    void collectDropsNear(const cocos2d::Vec2& playerWorldPos, Game::Inventory* inv) override;

    // 从农场进入房间的出生点（城镇无此门，返回零向量）。
    cocos2d::Vec2 farmRoomDoorSpawnPos() const override { return cocos2d::Vec2::ZERO; }
    // 从农场进入矿洞的出生点（城镇无此门，返回零向量）。
    cocos2d::Vec2 farmMineDoorSpawnPos() const override { return cocos2d::Vec2::ZERO; }

    // 获取箱子列表（城镇使用全局存档容器）。
    std::vector<Game::Chest>& chests() override { return _chests; }
    // 获取箱子列表（只读）。
    const std::vector<Game::Chest>& chests() const override { return _chests; }

    // 记录最近一次点击的世界坐标（用于三格选择）。
    void setLastClickWorldPos(const cocos2d::Vec2& p) override;
    // 清除最近一次点击记录。
    void clearLastClickWorldPos() override { _hasLastClick = false; }

    // 获取底层 TownMap 指针。
    Game::TownMap* getTownMap() const { return _map; }

    // 获取熔炉控制器指针（由地图控制器持有/挂接）。
    Controllers::FurnaceController* furnaceController() const { return _furnaceController; }

private:
    Game::TownMap* _map = nullptr;
    cocos2d::Node* _worldNode = nullptr;
    cocos2d::Vec2 _origin = cocos2d::Vec2::ZERO;
    int _cols = 0;
    int _rows = 0;
    std::vector<Game::TileType> _tiles;
    std::vector<Game::Chest> _chests;
    cocos2d::DrawNode* _cursor = nullptr;
    cocos2d::DrawNode* _chestDraw = nullptr;
    Controllers::DropSystem _dropSystem;
    cocos2d::Vec2 _lastClickWorldPos = cocos2d::Vec2::ZERO;
    bool _hasLastClick = false;
    Controllers::FurnaceController* _furnaceController = nullptr;
};

} // namespace Controllers
