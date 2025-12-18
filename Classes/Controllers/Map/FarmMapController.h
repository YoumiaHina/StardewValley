/**
 * FarmMapController: 农场地图控制器，负责 TMX 地图、瓦片、光标、门口与箱子检测、农作物与掉落。
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
#include "Game/Drop.h"
#include "Game/WorldState.h"
#include "Controllers/Environment/TreeSystem.h"
#include "Controllers/Environment/RockSystem.h"
#include "Controllers/Environment/EnvironmentObstacleSystemBase.h"
#include "Controllers/Interact/TileSelector.h"
#include "Controllers/Systems/ChestController.h"
#include "Controllers/Systems/FurnaceController.h"

namespace Controllers {

class FarmMapController : public Controllers::IMapController {
public:
    FarmMapController(cocos2d::Node* worldNode);

    void init();

    // IMapController overrides
    cocos2d::Vec2 getPlayerPosition(const cocos2d::Vec2& playerMapLocalPos) const override;
    cocos2d::Size getContentSize() const override;
    cocos2d::Vec2 getOrigin() const override;
    cocos2d::Vec2 clampPosition(const cocos2d::Vec2& current,
                                 const cocos2d::Vec2& next,
                                 float radius) const override;
    bool collides(const cocos2d::Vec2& pos, float radius) const override;

    bool isNearDoor(const cocos2d::Vec2& playerWorldPos) const override;
    bool isNearMineDoor(const cocos2d::Vec2& playerWorldPos) const override;
    bool isNearBeachDoor(const cocos2d::Vec2& playerWorldPos) const override;
    bool isNearTownDoor(const cocos2d::Vec2& playerWorldPos) const override;
    bool isNearChest(const cocos2d::Vec2& playerWorldPos) const override;

    float tileSize() const override;
    bool inBounds(int c, int r) const override;
    std::pair<int,int> targetTile(const cocos2d::Vec2& playerPos,
                                  const cocos2d::Vec2& lastDir) const override;
    void updateCursor(const cocos2d::Vec2& playerPos,
                      const cocos2d::Vec2& lastDir) override;

    Game::TileType getTile(int c, int r) const override;
    void setTile(int c, int r, Game::TileType t) override;
    cocos2d::Vec2 tileToWorld(int c, int r) const override;
    void worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const override;

    EnvironmentObstacleSystemBase* obstacleSystem(ObstacleKind kind) override;
    const EnvironmentObstacleSystemBase* obstacleSystem(ObstacleKind kind) const override;

    void refreshMapVisuals() override;
    void refreshCropsVisuals() override;
    void refreshDropsVisuals() override;
    void spawnDropAt(int c, int r, int itemType, int qty) override;

    const std::vector<Game::Chest>& chests() const override;
    std::vector<Game::Chest>& chests() override;

    Controllers::FurnaceController* furnaceController() const { return _furnaceController; }

    Game::FarmMap* tmx() const;
    cocos2d::Node* worldNode() const;
    void addActorToMap(cocos2d::Node* node, int zOrder) override;
    cocos2d::Vec2 farmMineDoorSpawnPos() const override;
    cocos2d::Vec2 farmRoomDoorSpawnPos() const override;
    cocos2d::Vec2 farmBeachDoorSpawnPos() const override;
    cocos2d::Vec2 farmTownDoorSpawnPos() const override;

    bool isNearLake(const cocos2d::Vec2& playerWorldPos, float radius) const override;
    void sortActorWithEnvironment(cocos2d::Node* actor) override;
    bool isFarm() const override;
    void setLastClickWorldPos(const cocos2d::Vec2& p) override;
    void clearLastClickWorldPos() override;

    void collectDropsNear(const cocos2d::Vec2& playerWorldPos, Game::Inventory* inv) override;

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
    std::vector<Game::Drop> _drops;
    cocos2d::DrawNode* _dropsDraw = nullptr;
    cocos2d::Node* _dropsRoot = nullptr;
    Controllers::ChestController* _chestController = nullptr;
    Controllers::FurnaceController* _furnaceController = nullptr;
    cocos2d::DrawNode* _cropsDraw = nullptr;
    cocos2d::Node* _cropsRoot = nullptr;
    std::unordered_map<long long, cocos2d::Sprite*> _cropSprites;
    std::unordered_map<long long, cocos2d::Sprite*> _cropSpritesTop;

    // Tilled tile overlay sprites
    cocos2d::Node* _tileRoot = nullptr;
    std::unordered_map<long long, cocos2d::Sprite*> _tileSprites;
    std::unordered_map<long long, cocos2d::Sprite*> _waterSprites;
    cocos2d::Node* _actorsRoot = nullptr;
    Controllers::EnvironmentObstacleSystemBase* _treeSystem = nullptr;
    Controllers::EnvironmentObstacleSystemBase* _rockSystem = nullptr;
    cocos2d::Vec2 _lastClickWorldPos = cocos2d::Vec2::ZERO;
    bool _hasLastClick = false;

    void applyStaticNotSoilMask();
};

}
// namespace Controllers
