#pragma once

#include "Controllers/IMapController.h"
#include "Game/Map/TownMap.h"

namespace Controllers {

class TownMapController : public IMapController {
public:
    explicit TownMapController(Game::TownMap* map, cocos2d::Node* worldNode);

    cocos2d::Vec2 getPlayerPosition(const cocos2d::Vec2& playerMapLocalPos) const override;

    void addActorToMap(cocos2d::Node* node, int zOrder) override;

    cocos2d::Size getContentSize() const override { return _map ? _map->getContentSize() : cocos2d::Size::ZERO; }
    cocos2d::Vec2 getOrigin() const override { return _origin; }

    float tileSize() const override { return _map ? _map->getTileSize().width : 0.0f; }
    void worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const override {
        if (_map) { _map->worldToTileIndex(p, c, r); } else { c = 0; r = 0; }
    }
    cocos2d::Vec2 tileToWorld(int c, int r) const override { return _map ? _map->tileToWorld(c,r) : cocos2d::Vec2(); }
    bool inBounds(int c, int r) const override {
        return _map && c>=0 && r>=0 && c < (int)_map->getMapSize().width && r < (int)_map->getMapSize().height;
    }

    cocos2d::Vec2 clampPosition(const cocos2d::Vec2& current,
                                const cocos2d::Vec2& next,
                                float radius) const override;
    bool collides(const cocos2d::Vec2& p, float radius) const override;

    bool isNearDoor(const cocos2d::Vec2& p) const override { return _map ? _map->nearDoorToFarm(p) : false; }
    bool isNearMineDoor(const cocos2d::Vec2& p) const override { return false; }
    bool isNearChest(const cocos2d::Vec2& p) const override;
    bool isNearLake(const cocos2d::Vec2& p, float radius) const override { return false; }

    Game::TileType getTile(int c, int r) const override { return Game::TileType::Soil; }

    std::pair<int,int> targetTile(const cocos2d::Vec2& p, const cocos2d::Vec2& lastDir) const override;
    void updateCursor(const cocos2d::Vec2& playerPos,
                      const cocos2d::Vec2& lastDir) override;

    void refreshMapVisuals() override;
    void refreshCropsVisuals() override {}

    cocos2d::Vec2 farmRoomDoorSpawnPos() const override { return cocos2d::Vec2::ZERO; }
    cocos2d::Vec2 farmMineDoorSpawnPos() const override { return cocos2d::Vec2::ZERO; }

    std::vector<Game::Chest>& chests() override { return _chests; }
    const std::vector<Game::Chest>& chests() const override { return _chests; }

    void setLastClickWorldPos(const cocos2d::Vec2& p) override { _lastClickWorldPos = p; _hasLastClick = true; }
    void clearLastClickWorldPos() override { _hasLastClick = false; }

    Game::TownMap* getTownMap() const { return _map; }

private:
    Game::TownMap* _map = nullptr;
    cocos2d::Node* _worldNode = nullptr;
    cocos2d::Vec2 _origin = cocos2d::Vec2::ZERO;
    std::vector<Game::Chest> _chests;
    cocos2d::DrawNode* _cursor = nullptr;
    cocos2d::DrawNode* _chestDraw = nullptr;
    cocos2d::Vec2 _lastClickWorldPos = cocos2d::Vec2::ZERO;
    bool _hasLastClick = false;
};

} // namespace Controllers
