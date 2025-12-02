/**
 * GameScene: Main game scene placeholder.
 */
#include "Scenes/GameScene.h"
#include "cocos2d.h"

USING_NS_CC;

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // Placeholder label
    auto label = Label::createWithTTF("Game Scene Placeholder", "fonts/Marker Felt.ttf", 30);
    if (label) {
        label->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height * 0.85f));
        this->addChild(label, 1);
    }

    // Simple placeholder player: a colored square
    auto player = DrawNode::create();
    const float size = 16.f;
    Vec2 verts[4] = { Vec2(-size, -size), Vec2(size, -size), Vec2(size, size), Vec2(-size, size) };
    player->drawSolidPoly(verts, 4, Color4F(0.2f, 0.7f, 0.9f, 1.0f));
    player->setPosition(Vec2(origin.x + visibleSize.width / 2,
                             origin.y + visibleSize.height / 2));
    this->addChild(player, 0);

    // Keyboard movement (placeholder)
    auto listener = EventListenerKeyboard::create();
    listener->onKeyPressed = [player](EventKeyboard::KeyCode code, Event*) {
        Vec2 p = player->getPosition();
        const float step = 12.f;
        switch (code) {
            case EventKeyboard::KeyCode::KEY_W:
            case EventKeyboard::KeyCode::KEY_UP_ARROW:    p.y += step; break;
            case EventKeyboard::KeyCode::KEY_S:
            case EventKeyboard::KeyCode::KEY_DOWN_ARROW:  p.y -= step; break;
            case EventKeyboard::KeyCode::KEY_A:
            case EventKeyboard::KeyCode::KEY_LEFT_ARROW:  p.x -= step; break;
            case EventKeyboard::KeyCode::KEY_D:
            case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: p.x += step; break;
            default: break;
        }
        player->setPosition(p);
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}