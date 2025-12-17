#pragma once

#include "cocos2d.h"
#include <vector>
#include <unordered_map>
#include "Game/Mineral.h"
#include "Controllers/Environment/EnvironmentObstacleSystemBase.h"
#include "Game/GameConfig.h"

namespace Controllers {

class MineMapController;

class MineralSystem : public EnvironmentObstacleSystemBase {
public:
    MineralSystem(MineMapController* map)
    : _map(map) {}

    void bindRuntimeStorage(std::vector<Game::MineralData>* minerals) { _runtime = minerals; }

    void clearVisuals();
    void syncVisuals();

    void generateNodesForFloor(std::vector<Game::MineralData>& outNodes,
                               const std::vector<cocos2d::Vec2>& candidates,
                               const std::vector<cocos2d::Vec2>& stairWorldPos) const;

    bool hitNearestNode(std::vector<Game::MineralData>& nodes,
                        const cocos2d::Vec2& worldPos,
                        int power) const;

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

private:
    MineMapController* _map = nullptr;
    cocos2d::Node* _root = nullptr;
    std::vector<Game::MineralData>* _runtime = nullptr;
    std::unordered_map<long long, Game::Mineral*> _obstacles;
};

}
