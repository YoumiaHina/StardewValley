#pragma once

#include "cocos2d.h"
#include <vector>
#include "Game/Chest.h"

namespace Controllers {

class ChestController {
public:
    explicit ChestController(bool isFarm)
    : _isFarm(isFarm) {}

    void attachTo(cocos2d::Node* parentNode, int zOrder);
    void syncLoad();

    const std::vector<Game::Chest>& chests() const;
    std::vector<Game::Chest>& chests();

    bool isNearChest(const cocos2d::Vec2& worldPos) const;
    bool collides(const cocos2d::Vec2& worldPos) const;

    void refreshVisuals();

private:
    bool _isFarm = true;
    cocos2d::Node* _parentNode = nullptr;
    cocos2d::DrawNode* _chestDraw = nullptr;
    std::vector<Game::Chest> _chests;
};

}

