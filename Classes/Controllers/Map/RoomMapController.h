/**
 * RoomMapController: 室内地图控制器，负责房间几何、门/床检测与箱子区域。
 */
#pragma once

#include "cocos2d.h"
#include <vector>
#include "Controllers/IMapController.h"
#include "Game/Chest.h"
#include "Game/Map/RoomMap.h"
#include "Controllers/Systems/ChestController.h"

namespace Controllers {

class RoomMapController : public Controllers::IMapController {
public:
    RoomMapController(cocos2d::Node* worldNode)
    : _worldNode(worldNode) {}

    void init();

    // IMapController overrides
    cocos2d::Vec2 getPlayerPosition(const cocos2d::Vec2& playerMapLocalPos) const override;
    cocos2d::Size getContentSize() const override;
    cocos2d::Vec2 getOrigin() const override { return cocos2d::Vec2(0,0); }
    cocos2d::Vec2 clampPosition(const cocos2d::Vec2& current,
                                 const cocos2d::Vec2& next,
                                 float radius) const override;
    bool collides(const cocos2d::Vec2& pos, float radius) const override;

    bool isNearDoor(const cocos2d::Vec2& playerWorldPos) const override;
    bool isNearFarmDoor(const cocos2d::Vec2& playerWorldPos) const override;
    bool isNearChest(const cocos2d::Vec2& playerWorldPos) const override;

    // Room 不涉及瓦片
    bool inBounds(int, int) const override { return false; }

    // 数据访问
    const cocos2d::Rect& roomRect() const { return _roomRect; }
    const cocos2d::Rect& doorRect() const { return _doorRect; }
    const cocos2d::Rect& bedRect()  const { return _bedRect; }
    const std::vector<Game::Chest>& chests() const override {
        static const std::vector<Game::Chest> empty;
        return _chestController ? _chestController->chests() : empty;
    }
    std::vector<Game::Chest>& chests() override {
        static std::vector<Game::Chest> empty;
        return _chestController ? _chestController->chests() : empty;
    }
    void addActorToMap(cocos2d::Node* node, int zOrder) override;
    cocos2d::Vec2 roomFarmDoorSpawnPos() const;
    void refreshChestsVisuals();

private:
    cocos2d::Node* _worldNode = nullptr;
    cocos2d::DrawNode* _roomDraw = nullptr;
    Game::RoomMap* _roomMap = nullptr;
    cocos2d::Rect _roomRect;
    cocos2d::Rect _doorRect;
    cocos2d::Rect _bedRect;
    Controllers::ChestController* _chestController = nullptr;
};

}
// namespace Controllers
