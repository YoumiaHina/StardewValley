/**
 * SplashScene: Simple boot scene showing a title and switching to menu.
 */
#include "Scenes/SplashScene.h"
#include "Scenes/MainMenuScene.h"
#include "cocos2d.h"

USING_NS_CC;

Scene* SplashScene::createScene() {
    return SplashScene::create();
}

bool SplashScene::init() {
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // Title
    auto title = Label::createWithTTF("Stardew Valley", "fonts/Marker Felt.ttf", 36);
    if (title) {
        title->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height * 0.65f));
        this->addChild(title, 1);
    }

    // Center image (reuse existing resource)
    auto sprite = Sprite::create("HelloWorld.png");
    if (sprite) {
        sprite->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height / 2));
        this->addChild(sprite, 0);
    }

    // Delay then go to main menu
    this->scheduleOnce(CC_SCHEDULE_SELECTOR(SplashScene::goToMainMenu), 1.5f);
    return true;
}

void SplashScene::goToMainMenu(float /*dt*/) {
    auto next = MainMenuScene::createScene();
    auto trans = TransitionFade::create(0.4f, next);
    Director::getInstance()->replaceScene(trans);
}