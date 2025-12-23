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

    auto sprite = Sprite::create("HelloWorld1.jpg");
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
