/**
 * MainMenuScene: Minimal main menu with Start/Quit.
 */
#include "Scenes/MainMenuScene.h"
#include "Scenes/RoomScene.h"
#include "Scenes/CustomizationScene.h"
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

    // Background image (placed at Resources/res/start_bg.png)
    // Scales to cover the entire screen while preserving aspect ratio.
    auto bg = Sprite::create("res/start_bg.png");
    if (bg) {
        bg->setAnchorPoint(Vec2(0.5f, 0.5f));
        bg->setPosition(Vec2(origin.x + visibleSize.width / 2,
                              origin.y + visibleSize.height / 2));
        const auto imgSize = bg->getContentSize();
        const float scaleX = visibleSize.width  / imgSize.width;
        const float scaleY = visibleSize.height / imgSize.height;
        bg->setScale(std::max(scaleX, scaleY)); // cover whole screen
        this->addChild(bg, 0);
    } else {
        // Fallback solid background if image is missing
        auto solid = LayerColor::create(Color4B(26, 39, 63, 255));
        this->addChild(solid, 0);
    }

    // Soft shade to improve text readability on bright images
    auto shade = LayerColor::create(Color4B(0, 0, 0, 60));
    this->addChild(shade, 0);

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
    auto nextScene = CustomizationScene::createScene();
    auto trans = TransitionFade::create(0.5f, nextScene);
    Director::getInstance()->replaceScene(trans);
}

void MainMenuScene::onExit(Ref* /*sender*/) {
    Director::getInstance()->end();
}