#pragma once

#include "cocos2d.h"
#include <unordered_map>
#include <functional>
#include "Game/Rock.h"
#include "Game/Map/MapBase.h"
#include "Game/Tile.h"
#include "Controllers/Environment/EnvironmentObstacleSystemBase.h"

namespace Controllers {

class RockSystem : public EnvironmentObstacleSystemBase {
public:
    void attachTo(cocos2d::Node* root) override;

    bool spawnFromTile(int c, int r, const cocos2d::Vec2& tileCenter,
                       Game::MapBase* map, int tileSize) override;

    void spawnRandom(int count, int cols, int rows,
                     const std::function<cocos2d::Vec2(int,int)>& tileToWorld,
                     Game::MapBase* map, int tileSize,
                     const std::function<bool(int,int)>& isSafe) override;

    Game::Rock* findRockAt(int c, int r) const;

    bool collides(const cocos2d::Vec2& point, float radius, int tileSize) const override;

    bool damageAt(int c, int r, int amount,
                  const std::function<void(int,int,int)>& spawnDrop,
                  const std::function<void(int,int, Game::TileType)>& setTile) override;

    void sortRocks();
    bool isEmpty() const override;
    std::vector<Game::RockPos> getAllRockTiles() const;

private:
    cocos2d::Node* _root = nullptr;
    std::unordered_map<long long, Game::Rock*> _rocks;
};

}
