/**
 * RoomMapController: 室内地图控制器，负责房间几何、门/床检测与箱子区域。
 */
#pragma once

#include "cocos2d.h"
#include <vector>
#include "Controllers/Map/IMapController.h"
#include "Game/Chest.h"
#include "Game/Map/RoomMap.h"
#include "Controllers/Systems/ChestController.h"

namespace Controllers {

class FurnaceController;

/**
 * RoomMapController：室内地图控制器。
 * - 职责：加载室内 TMX、维护房间几何（房间/门/床区域）、提供坐标换算与交互光标、门与箱子检测、以及室内箱子/熔炉等系统挂接入口。
 * - 职责边界：不在此处实现背包/掉落等室外业务规则；箱子/熔炉交互由对应 Controller 负责，地图控制器只提供空间与转发接口。
 * - 协作对象：通过 IMapController 接口与 PlayerController/ToolSystem，以及 ChestController/FurnaceController 等模块协作。
 */
class RoomMapController : public Controllers::IMapController {
public:
    // 构造：绑定世界节点。
    explicit RoomMapController(cocos2d::Node* worldNode);

    // 初始化：创建 TMX、解析门/床区域、挂接箱子/熔炉控制器等。
    void init();

    // IMapController overrides
    // 将玩家在地图局部坐标转换为世界节点坐标系位置。
    cocos2d::Vec2 getPlayerPosition(const cocos2d::Vec2& playerMapLocalPos) const override;
    // 获取地图内容尺寸（像素）。
    cocos2d::Size getContentSize() const override;
    // 获取地图原点偏移（用于相机计算）。
    cocos2d::Vec2 getOrigin() const override { return cocos2d::Vec2(0, 0); }
    // 将玩家下一帧位置夹紧到房间边界/碰撞规则允许范围内。
    cocos2d::Vec2 clampPosition(const cocos2d::Vec2& current,
                                 const cocos2d::Vec2& next,
                                 float radius) const override;
    // 检测指定位置是否与房间/箱子/熔炉发生碰撞。
    bool collides(const cocos2d::Vec2& pos, float radius) const override;

    // 是否靠近室内门区域。
    bool isNearDoor(const cocos2d::Vec2& playerWorldPos) const override;
    // 是否靠近返回农场的门（若 TMX 提供 DoorToFarm 区域则使用之）。
    bool isNearFarmDoor(const cocos2d::Vec2& playerWorldPos) const override;
    // 是否靠近箱子交互区域。
    bool isNearChest(const cocos2d::Vec2& playerWorldPos) const override;

    // 获取瓦片尺寸（像素）。
    float tileSize() const override { return 16.0f; }
    // 世界坐标转瓦片索引。
    void worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const override;
    // 瓦片索引转世界坐标（瓦片中心点）。
    cocos2d::Vec2 tileToWorld(int c, int r) const override;
    // 检查瓦片索引是否在边界内。
    bool inBounds(int c, int r) const override { return c >= 0 && r >= 0 && c < _cols && r < _rows; }

    // 获取瓦片类型（室内默认不可耕作）。
    Game::TileType getTile(int c, int r) const override;
    // 设置瓦片类型（室内固定为不可耕作）。
    void setTile(int c, int r, Game::TileType t) override;

    // 计算玩家面向/点击目标瓦片索引。
    std::pair<int,int> targetTile(const cocos2d::Vec2& p, const cocos2d::Vec2& lastDir) const override;
    // 更新交互光标（扇形选择）。
    void updateCursor(const cocos2d::Vec2& playerPos,
                      const cocos2d::Vec2& lastDir) override;

    // 记录最近一次点击的世界坐标（用于三格选择）。
    void setLastClickWorldPos(const cocos2d::Vec2& p) override;
    // 清除最近一次点击记录。
    void clearLastClickWorldPos() override { _hasLastClick = false; }

    // 数据访问
    // 获取房间整体矩形区域。
    const cocos2d::Rect& roomRect() const { return _roomRect; }
    // 获取室内门区域。
    const cocos2d::Rect& doorRect() const { return _doorRect; }
    // 获取床区域。
    const cocos2d::Rect& bedRect() const { return _bedRect; }
    // 获取箱子列表（只读）。
    const std::vector<Game::Chest>& chests() const override;
    // 获取箱子列表（可写）。
    std::vector<Game::Chest>& chests() override;
    // 将角色节点挂到正确父节点与层级。
    void addActorToMap(cocos2d::Node* node, int zOrder) override;
    // 获取从房间返回农场的出生点。
    cocos2d::Vec2 roomFarmDoorSpawnPos() const;
    // 刷新箱子可视（绘制/精灵同步）。
    void refreshChestsVisuals();

    // 获取熔炉控制器指针（由房间控制器持有/挂接）。
    Controllers::FurnaceController* furnaceController() const { return _furnaceController; }

private:
    cocos2d::Node* _worldNode = nullptr;
    cocos2d::DrawNode* _roomDraw = nullptr;
    Game::RoomMap* _roomMap = nullptr;
    int _cols = 0;
    int _rows = 0;
    std::vector<Game::TileType> _tiles;
    cocos2d::DrawNode* _cursor = nullptr;
    cocos2d::Vec2 _lastClickWorldPos = cocos2d::Vec2::ZERO;
    bool _hasLastClick = false;
    cocos2d::Rect _roomRect;
    cocos2d::Rect _doorRect;
    cocos2d::Rect _bedRect;
    Controllers::ChestController* _chestController = nullptr;
    Controllers::FurnaceController* _furnaceController = nullptr;
};

}
// namespace Controllers
