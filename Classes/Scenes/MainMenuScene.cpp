/**
 * MainMenuScene: Minimal main menu with Start/Quit.
 */
#include "Scenes/MainMenuScene.h"
#include "Scenes/RoomScene.h"
#include "Scenes/FarmScene.h"
#include "Scenes/MineScene.h"
#include "Scenes/BeachScene.h"
#include "Scenes/TownScene.h"
#include "Scenes/CustomizationScene.h"
#include "Game/WorldState.h"
#include "Game/Save/SaveSystem.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"

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

    auto startLabel = Label::createWithTTF("New Game", "fonts/Marker Felt.ttf", 28);
    auto loadLabel  = Label::createWithTTF("Load Game", "fonts/Marker Felt.ttf", 28);
    auto exitLabel  = Label::createWithTTF("Quit", "fonts/Marker Felt.ttf", 28);

    auto startItem = MenuItemLabel::create(startLabel, CC_CALLBACK_1(MainMenuScene::onStart, this));
    auto loadItem  = MenuItemLabel::create(loadLabel,  CC_CALLBACK_1(MainMenuScene::onLoad, this));
    auto exitItem  = MenuItemLabel::create(exitLabel,  CC_CALLBACK_1(MainMenuScene::onExit, this));

    auto menu = Menu::create(startItem, loadItem, exitItem, nullptr);
    menu->alignItemsVerticallyWithPadding(18.0f);
    menu->setPosition(Vec2(origin.x + visibleSize.width / 2,
                           origin.y + visibleSize.height * 0.45f));
    this->addChild(menu, 1);

    return true;
}

void MainMenuScene::onStart(Ref* sender) {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    auto overlay = LayerColor::create(Color4B(0, 0, 0, 180));
    addChild(overlay, 2);
    auto overlayListener = EventListenerTouchOneByOne::create();
    overlayListener->setSwallowTouches(true);
    overlayListener->onTouchBegan = [](Touch*, Event*) { return true; };
    overlay->getEventDispatcher()->addEventListenerWithSceneGraphPriority(overlayListener, overlay);
    auto label = Label::createWithTTF("Enter Save Name", "fonts/Marker Felt.ttf", 30);
    if (label) {
        label->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                origin.y + visibleSize.height * 0.7f));
        overlay->addChild(label, 1);
    }

    float boxW = visibleSize.width * 0.5f;
    float boxH = 80.0f;
    auto inputBg = LayerColor::create(Color4B(0, 0, 0, 220), boxW, boxH);
    inputBg->setPosition(Vec2(origin.x + (visibleSize.width - boxW) * 0.5f,
                              origin.y + visibleSize.height * 0.5f - boxH * 0.5f));
    overlay->addChild(inputBg, 1);

    auto textField = cocos2d::ui::TextField::create("Save Name", "fonts/Marker Felt.ttf", 24);
    textField->setMaxLengthEnabled(true);
    textField->setMaxLength(32);
    textField->setTextColor(Color4B::WHITE);
    textField->setCursorEnabled(true);
    textField->setCursorChar('|');
    textField->setPosition(Vec2(boxW * 0.5f, boxH * 0.5f));
    inputBg->addChild(textField, 1);
    textField->setAttachWithIME(true);

    auto okLabel = Label::createWithTTF("OK", "fonts/Marker Felt.ttf", 24);
    auto cancelLabel = Label::createWithTTF("Cancel", "fonts/Marker Felt.ttf", 24);

    auto okItem = MenuItemLabel::create(okLabel, [this, overlay, textField](Ref*) {
        std::string name = textField->getString();
        if (name.empty()) {
            name = "NewSave";
        }
        std::string path = Game::makeSavePathWithName(name);
        Game::setCurrentSavePath(path);
        auto& ws = Game::globalState();
        ws = Game::WorldState();
        ws.lastScene = static_cast<int>(Game::SceneKind::Room);
        auto nextScene = CustomizationScene::createScene();
        auto trans = TransitionFade::create(0.5f, nextScene);
        Director::getInstance()->replaceScene(trans);
    });

    auto cancelItem = MenuItemLabel::create(cancelLabel, [overlay](Ref*) {
        overlay->removeFromParent();
    });

    auto menu = Menu::create(okItem, cancelItem, nullptr);
    menu->alignItemsVerticallyWithPadding(10.0f);
    menu->setPosition(Vec2(origin.x + visibleSize.width / 2,
                           origin.y + visibleSize.height * 0.35f));
    overlay->addChild(menu, 1);
}

static cocos2d::Scene* createSceneForLastState() {
    auto& ws = Game::globalState();
    auto kind = static_cast<Game::SceneKind>(ws.lastScene);
    switch (kind) {
        case Game::SceneKind::Farm:
            return FarmScene::createScene();
        case Game::SceneKind::Mine:
            return MineScene::createScene();
        case Game::SceneKind::Beach:
            return BeachScene::createScene();
        case Game::SceneKind::Town:
            return TownScene::createScene();
        case Game::SceneKind::Room:
        default:
            return RoomScene::createScene();
    }
}

void MainMenuScene::onLoad(Ref* sender) {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    auto overlay = LayerColor::create(Color4B(0, 0, 0, 180));
    addChild(overlay, 2);
    auto overlayListener = EventListenerTouchOneByOne::create();
    overlayListener->setSwallowTouches(true);
    overlayListener->onTouchBegan = [](Touch*, Event*) { return true; };
    overlay->getEventDispatcher()->addEventListenerWithSceneGraphPriority(overlayListener, overlay);

    auto title = Label::createWithTTF("Select Save", "fonts/Marker Felt.ttf", 30);
    if (title) {
        title->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                origin.y + visibleSize.height * 0.7f));
        overlay->addChild(title, 1);
    }

    auto* fu = FileUtils::getInstance();
    std::string dir = Game::saveDirectory();
    std::vector<std::string> paths = fu->listFiles(dir);

    cocos2d::Vector<MenuItem*> items;
    for (const auto& p : paths) {
        if (p.size() < 4) continue;
        if (p.substr(p.size() - 4) != ".txt") continue;
        std::string name = p;
        auto pos = name.find_last_of("/\\");
        if (pos != std::string::npos) {
            name = name.substr(pos + 1);
        }
        auto label = Label::createWithTTF(name, "fonts/Marker Felt.ttf", 24);
        auto item = MenuItemLabel::create(label, [overlay, p](Ref*){
            if (!Game::loadFromFile(p)) {
                overlay->removeFromParent();
                return;
            }
            auto nextScene = createSceneForLastState();
            auto trans = TransitionFade::create(0.5f, nextScene);
            Director::getInstance()->replaceScene(trans);
        });
        items.pushBack(item);
    }

    if (!items.empty()) {
        auto menu = Menu::createWithArray(items);
        menu->alignItemsVerticallyWithPadding(10.0f);
        menu->setPosition(Vec2(origin.x + visibleSize.width / 2,
                               origin.y + visibleSize.height * 0.5f));
        overlay->addChild(menu, 1);
    } else {
        auto noLabel = Label::createWithTTF("No saves found", "fonts/Marker Felt.ttf", 24);
        if (noLabel) {
            noLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                      origin.y + visibleSize.height * 0.55f));
            overlay->addChild(noLabel, 1);
        }
    }

    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 24);
    auto backItem = MenuItemLabel::create(backLabel, [overlay](Ref*){
        overlay->removeFromParent();
    });
    auto backMenu = Menu::create(backItem, nullptr);
    backMenu->setPosition(Vec2(origin.x + visibleSize.width / 2,
                               origin.y + visibleSize.height * 0.2f));
    overlay->addChild(backMenu, 1);
}

void MainMenuScene::onExit(Ref* sender) {
    Director::getInstance()->end();
}
