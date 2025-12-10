#include "Controllers/Systems/FishingController.h"
#include "Game/GameConfig.h"
#include "Game/Item.h"

using namespace cocos2d;

namespace Controllers {

void FishingController::buildOverlayAt(const Vec2& worldPos) {
    if (_overlay) destroyOverlay();
    if (!_scene) return;
    _overlay = Node::create();
    Vec2 pos = worldPos; if (_worldNode) pos = _worldNode->convertToWorldSpace(worldPos);
    _overlay->setPosition(pos + Vec2(140.0f, 40.0f));
    _scene->addChild(_overlay, 5);

    // Background image (Fishing game map)
    _bgSprite = Sprite::create("fish/Fishing game map.png");
    if (_bgSprite) {
        _bgSprite->setAnchorPoint(Vec2(0.5f, 0.5f));
        _bgSprite->setPosition(Vec2(0, 0));
        auto vs = Director::getInstance()->getVisibleSize();
        // Scale to occupy ~60% of screen width/height while preserving aspect
        auto texSize = _bgSprite->getContentSize();
        float targetW = vs.width * 0.6f;
        float targetH = vs.height * 0.6f;
        float sx = targetW / texSize.width;
        float sy = targetH / texSize.height;
        float s = std::min(sx, sy);
        _bgSprite->setScale(s);
        _overlay->addChild(_bgSprite, 0);
    }

    _barBg = DrawNode::create();
    _overlay->addChild(_barBg);
    float w = 36.0f; float h = _barHeight;
    Vec2 v[4] = { Vec2(-w/2,-h/2), Vec2(w/2,-h/2), Vec2(w/2,h/2), Vec2(-w/2,h/2) };
    _barBg->drawSolidPoly(v, 4, Color4F(0.f,0.f,0.f,0.45f));
    _barBg->drawLine(v[0], v[1], Color4F(1,1,1,0.5f));
    _barBg->drawLine(v[1], v[2], Color4F(1,1,1,0.5f));
    _barBg->drawLine(v[2], v[3], Color4F(1,1,1,0.5f));
    _barBg->drawLine(v[3], v[0], Color4F(1,1,1,0.5f));

    _barCatch = DrawNode::create();
    _overlay->addChild(_barCatch);
    _progressFill = DrawNode::create();
    _overlay->addChild(_progressFill);
    _progressLabel = Label::createWithTTF("Catch 0%", "fonts/Marker Felt.ttf", 18);
    _progressLabel->setPosition(Vec2(0, h/2 + 22));
    _overlay->addChild(_progressLabel);

    // Use Bream as fish sprite
    _fishSprite = Sprite::create("fish/Bream.png");
    if (_fishSprite) { _fishSprite->setScale(0.6f); _overlay->addChild(_fishSprite, 1); }

    _kb = EventListenerKeyboard::create();
    _kb->onKeyPressed = [this](EventKeyboard::KeyCode code, Event*){
        if (!_active) return; if (code == EventKeyboard::KeyCode::KEY_SPACE) { _hold = true; }
    };
    _kb->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*){
        if (!_active) return; if (code == EventKeyboard::KeyCode::KEY_SPACE) { _hold = false; }
    };
    if (_scene) _scene->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_kb, _overlay);

    _mouse = EventListenerMouse::create();
    _mouse->onMouseDown = [this](EventMouse* e){ if (!_active) return; if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) _hold = true; };
    _mouse->onMouseUp =   [this](EventMouse* e){ if (!_active) return; if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) _hold = false; };
    if (_scene) _scene->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_mouse, _overlay);
}

void FishingController::destroyOverlay() {
    if (_kb && _scene) { _scene->getEventDispatcher()->removeEventListener(_kb); }
    if (_mouse && _scene) { _scene->getEventDispatcher()->removeEventListener(_mouse); }
    _kb = nullptr; _mouse = nullptr;
    if (_overlay) { _overlay->removeFromParent(); _overlay = nullptr; }
    _barBg = nullptr; _barCatch = nullptr; _progressFill = nullptr; _progressLabel = nullptr; _fishSprite = nullptr;
}

void FishingController::startAt(const Vec2& worldPos) {
    if (_active) return;
    float s = _map ? _map->tileSize() : static_cast<float>(GameConfig::TILE_SIZE);
    float radius = s * GameConfig::LAKE_REFILL_RADIUS_TILES;
    if (_map && !_map->isNearLake(worldPos, radius)) {
        if (_ui) _ui->popTextAt(worldPos, "Need water", Color3B::RED);
        return;
    }
    _active = true; _progress = 0.0f; _timeLeft = 12.0f; _barCatchPos = 80.0f; _barCatchVel = 0.0f; _fishPos = 140.0f; _fishVel = 0.0f; _hold = false;
    buildOverlayAt(worldPos);
    if (_ui) _ui->popTextAt(worldPos, "Fishing...", Color3B::YELLOW);
}

void FishingController::startAnywhere(const Vec2& worldPos) {
    if (_active) return;
    _active = true;
    _progress = 0.0f;
    _timeLeft = 12.0f;
    _barCatchPos = 80.0f;
    _barCatchVel = 0.0f;
    _fishPos = 140.0f;
    _fishVel = 0.0f;
    _hold = false;
    buildOverlayAt(worldPos);
    if (_overlay && _scene) {
        auto vs = cocos2d::Director::getInstance()->getVisibleSize();
        auto org = cocos2d::Director::getInstance()->getVisibleOrigin();
        _overlay->setPosition(org + cocos2d::Vec2(vs.width * 0.5f, vs.height * 0.5f));
    }
    if (_ui) _ui->popTextAt(worldPos, "Fishing...", Color3B::YELLOW);
}

void FishingController::update(float dt) {
    if (!_active) return;
    // physics for catch bar
    float accelUp = 240.0f; float gravity = 280.0f; float damping = 0.98f;
    _barCatchVel += (_hold ? accelUp : -gravity) * dt;
    _barCatchVel *= damping;
    _barCatchPos += _barCatchVel * dt;
    if (_barCatchPos < 0) { _barCatchPos = 0; _barCatchVel = 0; }
    if (_barCatchPos > _barHeight) { _barCatchPos = _barHeight; _barCatchVel = 0; }

    // fish movement (random jitter)
    float fishAccel = ((std::rand() % 200) - 100) * 0.8f; // -80..80
    _fishVel += fishAccel * dt;
    _fishVel = std::max(-140.0f, std::min(140.0f, _fishVel));
    _fishPos += _fishVel * dt;
    if (_fishPos < 20) { _fishPos = 20; _fishVel = std::abs(_fishVel); }
    if (_fishPos > _barHeight - 20) { _fishPos = _barHeight - 20; _fishVel = -std::abs(_fishVel); }

    // overlap -> progress
    float catchSize = 54.0f; // green bar height
    float diff = std::abs((_barCatchPos) - (_fishPos));
    bool overlap = diff <= catchSize * 0.5f;
    float gain = overlap ? 28.0f : -18.0f;
    _progress = std::max(0.0f, std::min(100.0f, _progress + gain * dt));
    _timeLeft -= dt;

    // draw
    if (_barCatch) {
        _barCatch->clear();
        float w = 28.0f; float h = catchSize;
        Vec2 v[4] = { Vec2(-w/2, -h/2), Vec2(w/2, -h/2), Vec2(w/2, h/2), Vec2(-w/2, h/2) };
        _barCatch->drawSolidPoly(v, 4, Color4F(0.15f, 0.85f, 0.35f, 0.9f));
        _barCatch->setPosition(Vec2(0, _barCatchPos - _barHeight*0.5f));
    }
    if (_progressFill) {
        _progressFill->clear();
        float pw = 140.0f, ph = 16.0f; float ratio = _progress / 100.0f; float fillW = pw * ratio;
        Vec2 bl(-pw/2, -_barHeight/2 - 28), br(-pw/2 + fillW, -_barHeight/2 - 28), tr(-pw/2 + fillW, -_barHeight/2 - 12), tl(-pw/2, -_barHeight/2 - 12);
        Vec2 rect[4] = { bl, br, tr, tl };
        _progressFill->drawSolidPoly(rect, 4, Color4F(0.2f, 0.6f, 1.0f, 0.85f));
    }
    if (_progressLabel) {
        _progressLabel->setString(StringUtils::format("Catch %d%%", static_cast<int>(_progress + 0.5f)));
    }
    if (_fishSprite) {
        _fishSprite->setPosition(Vec2(0, _fishPos - _barHeight*0.5f));
    }

    if (_progress >= 100.0f) {
        Vec2 posWorld = _worldNode ? _worldNode->convertToNodeSpace(_overlay->getPosition()) : _overlay->getPosition();
        onSuccess(posWorld);
    } else if (_timeLeft <= 0.0f) {
        Vec2 posWorld = _worldNode ? _worldNode->convertToNodeSpace(_overlay->getPosition()) : _overlay->getPosition();
        onFail(posWorld);
    }
}

void FishingController::onSuccess(const Vec2& worldPos) {
    _active = false;
    destroyOverlay();
    if (_inventory) {
        _inventory->addItems(Game::ItemType::Fish, 1);
    }
    if (_ui) {
        _ui->refreshHotbar();
        _ui->popTextAt(worldPos, "Caught a fish!", Color3B::GREEN);
    }
}

void FishingController::onFail(const Vec2& worldPos) {
    _active = false;
    destroyOverlay();
    if (_ui) {
        _ui->popTextAt(worldPos, "Fish escaped", Color3B::RED);
    }
}

void FishingController::cancel() {
    if (!_active) return;
    _active = false;
    destroyOverlay();
}
// namespace Controllers
}
