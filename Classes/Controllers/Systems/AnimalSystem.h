#pragma once

#include <vector>
#include "cocos2d.h"
#include "Game/Animal.h"
#include "Controllers/IMapController.h"

namespace Controllers {

class AnimalSystem {
public:
    AnimalSystem(Controllers::IMapController* map, cocos2d::Node* worldNode);

    void update(float dt);

    void advanceAnimalsDaily();

    bool tryFeedAnimal(const cocos2d::Vec2& playerPos, Game::ItemType feedType, int& consumedQty);

    void spawnAnimal(Game::AnimalType type, const cocos2d::Vec2& pos);

    bool buyAnimal(Game::AnimalType type, const cocos2d::Vec2& pos, long long price);

private:
    struct Instance {
        Game::Animal animal;
        cocos2d::Sprite* sprite = nullptr;
        cocos2d::Vec2 velocity;
        cocos2d::Label* growthLabel = nullptr;
        float idleTimer = 0.0f;
    };

    Controllers::IMapController* _map = nullptr;
    cocos2d::Node* _worldNode = nullptr;
    std::vector<Instance> _animals;

    void ensureSprite(Instance& inst);
    void updateGrowthLabel(Instance& inst);
};

// 离线每日推进：仅基于 WorldState，同步动物成长与产物到全局
void advanceAnimalsDailyWorldOnly();

}
