#pragma once

#include "cocos2d.h"
#include <vector>
#include <functional>
#include "Controllers/Environment/EnvironmentObstacleSystemBase.h"

namespace Controllers {

class MineMapController;

class StairSystem : public EnvironmentObstacleSystemBase {
public:
    StairSystem(MineMapController* map)
    : _map(map) {}

    void attachTo(cocos2d::Node* root) override;

    bool spawnFromTile(int c, int r, const cocos2d::Vec2& tileCenter,
                       Game::MapBase* map, int tileSize) override;

    void spawnRandom(int count, int cols, int rows,
                     const std::function<cocos2d::Vec2(int,int)>& tileToWorld,
                     Game::MapBase* map, int tileSize,
                     const std::function<bool(int,int)>& isSafe) override;

    bool collides(const cocos2d::Vec2& point, float radius, int tileSize) const override;

    bool damageAt(int c, int r, int amount,
                  const std::function<void(int,int,int)>& spawnDrop,
                  const std::function<void(int,int, Game::TileType)>& setTile) override;

    bool isEmpty() const override;

    void generateStairs(const std::vector<cocos2d::Vec2>& candidates,
                        int minCount,
                        int maxCount,
                        std::vector<cocos2d::Vec2>& outWorldPos) const;

private:
    MineMapController* _map = nullptr;
    cocos2d::Node* _root = nullptr;
};

}
