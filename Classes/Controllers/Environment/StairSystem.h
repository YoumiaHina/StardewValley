#pragma once

#include "cocos2d.h"
#include <vector>
#include <functional>
#include "Controllers/Environment/EnvironmentObstacleSystemBase.h"
#include "Game/Mineral.h"
#include "Game/Stair.h"

namespace Controllers {

class MineMapController;

class StairSystem : public EnvironmentObstacleSystemBase {
public:
    StairSystem(MineMapController* map)
    : _map(map) {}

    void attachTo(cocos2d::Node* root) override;

    void reset();

    bool spawnFromTile(int c, int r, const cocos2d::Vec2& tileCenter,
                       Game::IMapBase* map, int tileSize) override;

    void spawnRandom(int count, int cols, int rows,
                     const std::function<cocos2d::Vec2(int,int)>& tileToWorld,
                     Game::IMapBase* map, int tileSize,
                     const std::function<bool(int,int)>& isSafe) override;

    bool collides(const cocos2d::Vec2& point, float radius, int tileSize) const override;

    bool damageAt(int c, int r, int amount,
                  const std::function<void(int,int,int)>& spawnDrop,
                  const std::function<void(int,int, Game::TileType)>& setTile) override;

    bool isEmpty() const override;

    void generateStairs(const std::vector<cocos2d::Vec2>& candidates,
                        int minCount,
                        int maxCount,
                        std::vector<cocos2d::Vec2>& outWorldPos);

    void syncExtraStairsToMap(const std::vector<Game::MineralData>& minerals);

    void refreshVisuals();

private:
    struct StairData {
        cocos2d::Vec2 pos;
        Game::Stair* node = nullptr;
        bool covered = false;
    };

    MineMapController* _map = nullptr;
    cocos2d::Node* _root = nullptr;
    cocos2d::DrawNode* _debugDraw = nullptr;
    std::vector<StairData> _stairs;
};

}
