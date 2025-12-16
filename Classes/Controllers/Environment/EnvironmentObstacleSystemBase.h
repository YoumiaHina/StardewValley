#pragma once

#include "cocos2d.h"
#include <functional>
#include "Game/Map/MapBase.h"
#include "Game/Tile.h"

namespace Controllers {

class EnvironmentObstacleSystemBase {
public:
    virtual ~EnvironmentObstacleSystemBase() = default;

    virtual void attachTo(cocos2d::Node* root) = 0;

    virtual bool spawnFromTile(int c, int r, const cocos2d::Vec2& tileCenter,
                               Game::MapBase* map, int tileSize) = 0;

    virtual void spawnRandom(int count, int cols, int rows,
                             const std::function<cocos2d::Vec2(int,int)>& tileToWorld,
                             Game::MapBase* map, int tileSize,
                             const std::function<bool(int,int)>& isSafe) = 0;

    virtual bool collides(const cocos2d::Vec2& point, float radius, int tileSize) const = 0;

    virtual bool damageAt(int c, int r, int amount,
                          const std::function<void(int,int,int)>& spawnDrop,
                          const std::function<void(int,int, Game::TileType)>& setTile) = 0;

    virtual bool isEmpty() const = 0;
};

}
