#include "Controllers/UI/HUDUI.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"

using namespace cocos2d;

namespace Controllers {

void HUDUI::buildHUD() {
    auto &ws = Game::globalState();
    if (ws.maxEnergy <= 0) ws.maxEnergy = GameConfig::ENERGY_MAX;
    if (ws.energy < 0 || ws.energy > ws.maxEnergy) ws.energy = ws.maxEnergy;
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    if (!_hudTimeLabel) {
        _hudTimeLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 18);
        _hudTimeLabel->setColor(Color3B::WHITE);
        _hudTimeLabel->setAnchorPoint(Vec2(1,1));
        float pad = 10.0f;
        _hudTimeLabel->setPosition(Vec2(origin.x + visibleSize.width - pad, origin.y + visibleSize.height - pad));
        if (_scene) _scene->addChild(_hudTimeLabel, 3);
    }
    if (!_hudWeatherLabel) {
        _hudWeatherLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 18);
        _hudWeatherLabel->setColor(Color3B::WHITE);
        _hudWeatherLabel->setAnchorPoint(Vec2(1,1));
        float pad = 10.0f;
        _hudWeatherLabel->setPosition(Vec2(origin.x + visibleSize.width - pad, origin.y + visibleSize.height - pad - 24));
        if (_scene) _scene->addChild(_hudWeatherLabel, 3);
    }
    if (!_hudGoldLabel) {
        _hudGoldLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 18);
        _hudGoldLabel->setColor(Color3B::YELLOW);
        _hudGoldLabel->setAnchorPoint(Vec2(1,1));
        float pad = 10.0f;
        _hudGoldLabel->setPosition(Vec2(origin.x + visibleSize.width - pad, origin.y + visibleSize.height - pad - 48));
        if (_scene) _scene->addChild(_hudGoldLabel, 3);
    }
    if (!_energyNode) {
        _energyNode = Node::create();
        float pad = 10.0f;
        _energyNode->setPosition(Vec2(origin.x + visibleSize.width - pad, origin.y + pad));
        if (_scene) _scene->addChild(_energyNode, 3);
        float bw = 160.0f, bh = 18.0f;
        auto bg = DrawNode::create();
        Vec2 bl(-bw, 0), br(0, 0), tr(0, bh), tl(-bw, bh);
        Vec2 rect[4] = { bl, br, tr, tl };
        bg->drawSolidPoly(rect, 4, Color4F(0.f,0.f,0.f,0.35f));
        bg->drawLine(bl, br, Color4F(1,1,1,0.5f));
        bg->drawLine(br, tr, Color4F(1,1,1,0.5f));
        bg->drawLine(tr, tl, Color4F(1,1,1,0.5f));
        bg->drawLine(tl, bl, Color4F(1,1,1,0.5f));
        _energyNode->addChild(bg);
        _energyFill = DrawNode::create();
        _energyNode->addChild(_energyFill);
        _energyLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 16);
        _energyLabel->setAnchorPoint(Vec2(1,0.5f));
        _energyLabel->setPosition(Vec2(-4.0f, bh * 0.5f));
        _energyLabel->setColor(Color3B::WHITE);
        _energyNode->addChild(_energyLabel);
    }
    refreshHUD();
}

void HUDUI::refreshHUD() {
    auto &ws = Game::globalState();
    auto seasonName = [](int idx){
        switch (idx % 4) { case 0: return "Spring"; case 1: return "Summer"; case 2: return "Fall"; default: return "Winter"; }
    };
    if (_hudTimeLabel) {
        _hudTimeLabel->setString(StringUtils::format("%s Day %d, %02d:%02d", seasonName(ws.seasonIndex), ws.dayOfSeason, ws.timeHour, ws.timeMinute));
    }
    if (_hudWeatherLabel) {
        _hudWeatherLabel->setString(StringUtils::format("Weather: %s", ws.isRaining ? "Rain" : "Sunny"));
    }
    if (_hudGoldLabel) {
        _hudGoldLabel->setString(StringUtils::format("Gold: %lld", ws.gold));
    }
    if (_energyFill && _energyNode) {
        _energyFill->clear();
        float bw = 160.0f, bh = 18.0f;
        float ratio = ws.maxEnergy > 0 ? (static_cast<float>(ws.energy) / static_cast<float>(ws.maxEnergy)) : 0.f;
        ratio = std::max(0.f, std::min(1.f, ratio));
        float fillW = bw * ratio;
        Vec2 bl(-bw, 0), br(-bw + fillW, 0), tr(-bw + fillW, bh), tl(-bw, bh);
        Vec2 rect[4] = { bl, br, tr, tl };
        _energyFill->drawSolidPoly(rect, 4, Color4F(0.2f, 0.8f, 0.25f, 0.85f));
    }
    if (_energyLabel) {
        _energyLabel->setString(StringUtils::format("Energy %d/%d", ws.energy, ws.maxEnergy));
    }
    if (_hpFill && _hpNode) {
        _hpFill->clear();
        float bw = 160.0f, bh = 18.0f;
        float ratio = ws.maxHp > 0 ? (static_cast<float>(ws.hp) / static_cast<float>(ws.maxHp)) : 0.f;
        ratio = std::max(0.f, std::min(1.f, ratio));
        float fillW = bw * ratio;
        Vec2 bl(-bw, 0), br(-bw + fillW, 0), tr(-bw + fillW, bh), tl(-bw, bh);
        Vec2 rect[4] = { bl, br, tr, tl };
        _hpFill->drawSolidPoly(rect, 4, Color4F(0.9f, 0.15f, 0.15f, 0.95f));
    }
    if (_hpLabel) {
        _hpLabel->setString(StringUtils::format("HP %d/%d", ws.hp, ws.maxHp));
    }
}

void HUDUI::buildHPBarAboveEnergy() {
    if (!_energyNode || _hpNode) return;
    _hpNode = Node::create();
    Vec2 energyWorld = _energyNode->getPosition();
    float offsetY = 24.0f;
    _hpNode->setPosition(Vec2(energyWorld.x, energyWorld.y + offsetY));
    if (_scene) _scene->addChild(_hpNode, 3);
    float bw = 160.0f, bh = 18.0f;
    auto bg = DrawNode::create();
    Vec2 bl(-bw, 0), br(0, 0), tr(0, bh), tl(-bw, bh);
    Vec2 rect[4] = { bl, br, tr, tl };
    bg->drawSolidPoly(rect, 4, Color4F(0.f,0.f,0.f,0.35f));
    bg->drawLine(bl, br, Color4F(1,1,1,0.5f));
    bg->drawLine(br, tr, Color4F(1,1,1,0.5f));
    bg->drawLine(tr, tl, Color4F(1,1,1,0.5f));
    bg->drawLine(tl, bl, Color4F(1,1,1,0.5f));
    _hpNode->addChild(bg);
    _hpFill = DrawNode::create();
    _hpNode->addChild(_hpFill);
    _hpLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 16);
    _hpLabel->setAnchorPoint(Vec2(1,0.5f));
    _hpLabel->setPosition(Vec2(-4.0f, bh * 0.5f));
    _hpLabel->setColor(Color3B::RED);
    _hpNode->addChild(_hpLabel);
    refreshHUD();
}

void HUDUI::buildMineFloorLabel() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    if (!_mineFloorLabel) {
        _mineFloorLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 20);
        _mineFloorLabel->setColor(Color3B::WHITE);
        _mineFloorLabel->setAnchorPoint(Vec2(0,1));
        float pad = 10.0f;
        _mineFloorLabel->setPosition(Vec2(origin.x + pad, origin.y + visibleSize.height - pad));
        if (_scene) _scene->addChild(_mineFloorLabel, 3);
    }
}

void HUDUI::setMineFloorNumber(int floor) {
    if (_mineFloorLabel) {
        _mineFloorLabel->setString(StringUtils::format("Floor %d", floor));
    }
}

}
