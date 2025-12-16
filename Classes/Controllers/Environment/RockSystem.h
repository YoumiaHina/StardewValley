#pragma once

#include "cocos2d.h"
#include <unordered_map>
#include <functional>
#include "Game/Rock.h"
#include "Game/Map/FarmMap.h"
#include "Game/Tile.h"

namespace Controllers {

class RockSystem {
public:
    void attachTo(cocos2d::Node* root);

    bool spawnFromTile(int c, int r, const cocos2d::Vec2& tileCenter,
                       Game::FarmMap* map, int tileSize);

    void spawnRandom(int count, int cols, int rows,
                     const std::function<cocos2d::Vec2(int,int)>& tileToWorld,
                     Game::FarmMap* map, int tileSize,
                     const std::function<bool(int,int)>& isSafe);

    Game::Rock* findRockAt(int c, int r) const;

    bool collides(const cocos2d::Vec2& point, float radius, int tileSize) const;

    bool damageRockAt(int c, int r, int amount,
                      const std::function<void(int,int,int)>& spawnDrop,
                      const std::function<void(int,int, Game::TileType)>& setTile);

    void sortRocks();
    bool isEmpty() const;
    std::vector<Game::RockPos> getAllRockTiles() const;

private:
    cocos2d::Node* _root = nullptr;
    std::unordered_map<long long, Game::Rock*> _rocks;
};

}

