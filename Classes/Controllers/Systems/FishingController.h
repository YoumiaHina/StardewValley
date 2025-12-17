#pragma once

#include "cocos2d.h"
#include <memory>
#include "Controllers/IMapController.h"
#include "Game/Inventory.h"
#include "Controllers/UI/UIController.h"

namespace Controllers {

class FishingController {
public:
    FishingController(Controllers::IMapController* map,
                      std::shared_ptr<Game::Inventory> inventory,
                      Controllers::UIController* ui,
                      cocos2d::Scene* scene,
                      cocos2d::Node* worldNode)
    : _map(map), _inventory(std::move(inventory)), _ui(ui), _scene(scene), _worldNode(worldNode) {}

    void startAt(const cocos2d::Vec2& worldPos);
    void startAnywhere(const cocos2d::Vec2& worldPos);
    void update(float dt);
    bool isActive() const { return _active; }
    void cancel();
    void setMovementLocker(std::function<void(bool)> cb) { _setMovementLocked = std::move(cb); }

private:
    Controllers::IMapController* _map = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;
    Controllers::UIController* _ui = nullptr;
    cocos2d::Scene* _scene = nullptr;
    cocos2d::Node* _worldNode = nullptr;

    cocos2d::Node* _overlay = nullptr;
    cocos2d::DrawNode* _barBg = nullptr;
    cocos2d::DrawNode* _barCatch = nullptr;
    cocos2d::DrawNode* _progressFill = nullptr;
    cocos2d::Label* _progressLabel = nullptr;
    cocos2d::Sprite* _bgSprite = nullptr;
    cocos2d::Sprite* _fishSprite = nullptr;

    bool _active = false;
    float _barHeight = 220.0f;
    float _barCatchPos = 80.0f;
    float _barCatchVel = 0.0f;
    float _fishPos = 140.0f;
    float _fishVel = 0.0f;
    float _progress = 0.0f; // 0..100
    float _timeLeft = 12.0f; // seconds
    bool _hold = false; // input hold

    float _cooldown = 0.0f;

    cocos2d::EventListenerMouse* _mouse = nullptr;
    std::function<void(bool)> _setMovementLocked;

    void buildOverlayAt(const cocos2d::Vec2& worldPos);
    void destroyOverlay();
    void onSuccess(const cocos2d::Vec2& worldPos);
    void onFail(const cocos2d::Vec2& worldPos);
};

}
// namespace Controllers
