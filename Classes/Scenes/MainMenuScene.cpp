/**
 * MainMenuScene: Minimal main menu with Start/Quit.
 */
#include "Scenes/MainMenuScene.h"
#include "Scenes/GameScene.h"
#include "cocos2d.h"

USING_NS_CC;

Scene* MainMenuScene::createScene() {
    return MainMenuScene::create();
}

bool MainMenuScene::init() {
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // Title
    auto title = Label::createWithTTF("Stardew Valley", "fonts/Marker Felt.ttf", 40);
    if (title) {
        title->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height * 0.75f));
        this->addChild(title, 1);
    }

    // Menu items
    auto startLabel = Label::createWithTTF("Start Game", "fonts/Marker Felt.ttf", 28);
    auto exitLabel  = Label::createWithTTF("Quit", "fonts/Marker Felt.ttf", 28);

    auto startItem = MenuItemLabel::create(startLabel, CC_CALLBACK_1(MainMenuScene::onStart, this));
    auto exitItem  = MenuItemLabel::create(exitLabel,  CC_CALLBACK_1(MainMenuScene::onExit, this));

    auto menu = Menu::create(startItem, exitItem, nullptr);
    menu->alignItemsVerticallyWithPadding(18.0f);
    menu->setPosition(Vec2(origin.x + visibleSize.width / 2,
                           origin.y + visibleSize.height * 0.45f));
    this->addChild(menu, 1);

    return true;
}

void MainMenuScene::onStart(Ref* /*sender*/) {
    auto game = GameScene::createScene();
    auto trans = TransitionFade::create(0.3f, game);
    Director::getInstance()->replaceScene(trans);
}

void MainMenuScene::onExit(Ref* /*sender*/) {
    Director::getInstance()->end();
}