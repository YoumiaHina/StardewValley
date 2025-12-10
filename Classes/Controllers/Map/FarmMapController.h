/**
 * FarmMapController: 农场地图控制器，负责 TMX 地图、瓦片、光标、门口与箱子检测、农作物与掉落。
 */
#pragma once

#include "cocos2d.h"
#include <vector>
#include <utility>
#include <memory>
#include <unordered_map>
#include "Controllers/IMapController.h"
#include "Game/GameConfig.h"
#include "Game/FarmMap.h"
#include "Game/Chest.h"
#include "Game/Drop.h"
#include "Game/Crop.h"
#include "Game/WorldState.h"
#include "Game/Tree.h"
#include "Controllers/Environment/TreeSystem.h"

namespace Controllers {

class FarmMapController : public Controllers::IMapController {
public:
    FarmMapController(cocos2d::Node* worldNode)
    : _worldNode(worldNode) {}

    void init();

    // IMapController overrides
    cocos2d::Size getContentSize() const override;
    cocos2d::Vec2 getOrigin() const override { return _mapOrigin; }
    cocos2d::Vec2 clampPosition(const cocos2d::Vec2& current,
                                 const cocos2d::Vec2& next,
                                 float radius) const override;
    bool collides(const cocos2d::Vec2& pos, float radius) const override;

    bool isNearDoor(const cocos2d::Vec2& playerWorldPos) const override;
    bool isNearMineDoor(const cocos2d::Vec2& playerWorldPos) const override;
    bool isNearChest(const cocos2d::Vec2& playerWorldPos) const override;

    float tileSize() const override { return static_cast<float>(GameConfig::TILE_SIZE); }
    bool inBounds(int c, int r) const override;
    std::pair<int,int> targetTile(const cocos2d::Vec2& playerPos,
                                  const cocos2d::Vec2& lastDir) const override;
    void updateCursor(const cocos2d::Vec2& playerPos,
                      const cocos2d::Vec2& lastDir) override;

    Game::TileType getTile(int c, int r) const override;
    void setTile(int c, int r, Game::TileType t) override;
    cocos2d::Vec2 tileToWorld(int c, int r) const override;
    void worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const override;

    void refreshMapVisuals() override;
    void refreshCropsVisuals() override;
    void refreshDropsVisuals() override;
    void spawnDropAt(int c, int r, int itemType, int qty) override;

    // Expose data for other controllers
    const std::vector<Game::Chest>& chests() const { return _chests; }
    std::vector<Game::Chest>& chests() { return _chests; }

    Game::FarmMap* tmx() const { return _gameMap; }
    cocos2d::Node* worldNode() const { return _worldNode; }
    void addActorToMap(cocos2d::Node* node, int zOrder) override;
    cocos2d::Vec2 farmMineDoorSpawnPos() const override;
    cocos2d::Vec2 farmRoomDoorSpawnPos() const override;

private:
    cocos2d::Node* _worldNode = nullptr;
    cocos2d::Node* _mapNode = nullptr;
    Game::FarmMap* _gameMap = nullptr;
    int _cols = GameConfig::MAP_COLS;
    int _rows = GameConfig::MAP_ROWS;
    std::vector<Game::TileType> _tiles; // row-major
    cocos2d::DrawNode* _mapDraw = nullptr;
    cocos2d::DrawNode* _cursor = nullptr;
    cocos2d::Vec2 _mapOrigin;

    // Prompts/regions
    cocos2d::Rect _farmDoorRect;

    // Drops / Chests / Crops
    std::vector<Game::Drop> _drops;
    cocos2d::DrawNode* _dropsDraw = nullptr;
    std::vector<Game::Chest> _chests;
    cocos2d::DrawNode* _chestDraw = nullptr;
    cocos2d::DrawNode* _cropsDraw = nullptr;
    cocos2d::Node* _cropsRoot = nullptr;
    std::unordered_map<long long, cocos2d::Sprite*> _cropSprites;
    std::unordered_map<long long, cocos2d::Sprite*> _cropSpritesTop;

    // Tilled tile overlay sprites
    cocos2d::Node* _tileRoot = nullptr;
    std::unordered_map<long long, cocos2d::Sprite*> _tileSprites;
    cocos2d::Node* _actorsRoot = nullptr;
    Controllers::TreeSystem* _treeSystem = nullptr;


    // 接口扩展：湖边判定
public:
    bool isNearLake(const cocos2d::Vec2& playerWorldPos, float radius) const override;
    void sortActorWithEnvironment(cocos2d::Node* actor) override;
    bool damageTreeAt(int c, int r, int amount) override;
    Game::Tree* findTreeAt(int c, int r) const;
};

}
// namespace Controllers
