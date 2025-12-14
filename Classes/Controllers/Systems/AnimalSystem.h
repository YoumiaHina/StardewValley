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

    void spawnAnimal(Game::AnimalType type, const cocos2d::Vec2& pos);

private:
    struct Instance {
        Game::Animal animal;
        cocos2d::Sprite* sprite = nullptr;
        cocos2d::Vec2 velocity;
    };

    Controllers::IMapController* _map = nullptr;
    cocos2d::Node* _worldNode = nullptr;
    std::vector<Instance> _animals;

    void ensureSprite(Instance& inst);
};

}

