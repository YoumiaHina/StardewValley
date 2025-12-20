#include "Game/Tool/WaterCan.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/Tile.h"
#include "cocos2d.h"

using namespace cocos2d;

namespace {

int waterCanMaxForLevel(int lv) {
    int capped = lv;
    if (capped < 0) capped = 0;
    if (capped > 3) capped = 3;
    int cap = GameConfig::WATERING_CAN_MAX;
    for (int i = 0; i < capped; ++i) {
        cap *= 2;
    }
    return cap;
}

}

namespace Game {

WaterCan::~WaterCan() {
    detachHotbarOverlay();
}

ToolKind WaterCan::kind() const { return ToolKind::WaterCan; }
std::string WaterCan::displayName() const { return std::string("Water Can"); }

std::string WaterCan::iconPath() const {
    int lv = level();
    std::string prefix;
    if (lv == 1) {
        prefix = "copper_";
    } else if (lv == 2) {
        prefix = "iron_";
    } else if (lv >= 3) {
        prefix = "gold_";
    }
    return std::string("Tool/") + prefix + "WaterCan.png";
}

std::string WaterCan::use(Controllers::IMapController* map,
                          Controllers::CropSystem* crop,
                          std::function<Vec2()> getPlayerPos,
                          std::function<Vec2()> getLastDir,
                          Controllers::UIController* ui) {
    auto& ws = Game::globalState();
    int desiredMax = waterCanMaxForLevel(level());
    if (ws.maxWater != desiredMax) {
        if (ws.water > desiredMax) {
            ws.water = desiredMax;
        }
        ws.maxWater = desiredMax;
    }
    int need = GameConfig::ENERGY_COST_WATER;
    Vec2 playerPos = getPlayerPos ? getPlayerPos() : Vec2();
    float s = map->tileSize();
    float radius = s * GameConfig::LAKE_REFILL_RADIUS_TILES;
    bool nearLake = map->isNearLake(playerPos, radius);
    if (nearLake && ws.water < ws.maxWater) {
        ws.water = ws.maxWater;
        std::string msg = std::string("Refill! (") + std::to_string(ws.water) + std::string("/") + std::to_string(ws.maxWater) + std::string(")");
        need = 0;
        if (ui) {
            ui->refreshHUD();
            ui->refreshHotbar();
            map->refreshMapVisuals();
            map->refreshDropsVisuals();
            ui->popTextAt(map ? map->getPlayerPosition(playerPos) : playerPos, msg, Color3B::YELLOW);
        }
        refreshHotbarOverlay();
        return msg;
    }
    if (ws.energy < need) {
        if (getPlayerPos && ui) {
            Vec2 p = getPlayerPos();
            if (map) p = map->getPlayerPosition(p);
            ui->popTextAt(p, std::string("Not enough energy"), Color3B::RED);
        }
        return std::string("");
    }
    Vec2 lastDir = getLastDir ? getLastDir() : Vec2(0,-1);
    auto tgt = map->targetTile(playerPos, lastDir);
    int tc = tgt.first, tr = tgt.second;
    if (!map->inBounds(tc, tr)) return std::string("");
    auto current = map->getTile(tc, tr);
    std::string msg;
    if (current == Game::TileType::Tilled) {
        if (ws.water <= 0) {
            msg = nearLake ? std::string("Refill first") : std::string("No water");
        } else {
            map->setTile(tc, tr, Game::TileType::Watered);
            if (crop) { crop->markWateredAt(tc, tr); }
            ws.water = std::max(0, ws.water - GameConfig::WATERING_CAN_CONSUME);
            msg = std::string("Water! (") + std::to_string(ws.water) + std::string("/") + std::to_string(ws.maxWater) + std::string(")");
        }
    } else {
        msg = nearLake ? std::string("Hold to Refill") : std::string("Nothing");
    }
    ws.energy = std::max(0, ws.energy - need);
    if (ui) {
        ui->refreshHUD();
        ui->refreshHotbar();
        map->refreshMapVisuals();
        map->refreshDropsVisuals();
        ui->popTextAt(map ? map->getPlayerPosition(playerPos) : playerPos, msg, Color3B::YELLOW);
    }
    refreshHotbarOverlay();
    return msg;
}

void WaterCan::attachHotbarOverlay(cocos2d::Sprite* iconSprite, float cellW, float cellH) {
    if (!iconSprite) return;
    _iconSprite = iconSprite;
    _cellW = cellW;
    _cellH = cellH;
    auto parent = iconSprite->getParent();
    if (!parent) return;
    if (_waterBarNode && _waterBarNode->getParent()) {
        _waterBarNode->removeFromParent();
    }
    _waterBarNode = cocos2d::Node::create();
    _waterBarBg = cocos2d::DrawNode::create();
    _waterBarFill = cocos2d::DrawNode::create();
    _waterBarNode->addChild(_waterBarBg);
    _waterBarNode->addChild(_waterBarFill);
    parent->addChild(_waterBarNode, iconSprite->getLocalZOrder() + 1);

    float bw = _cellW;
    float bh = 8.0f;
    cocos2d::Vec2 bl(-bw/2, 0), br(bw/2, 0), tr(bw/2, bh), tl(-bw/2, bh);
    cocos2d::Vec2 rect[4] = { bl, br, tr, tl };
    _waterBarBg->drawSolidPoly(rect, 4, cocos2d::Color4F(0.f,0.f,0.f,0.35f));
    _waterBarBg->drawLine(bl, br, cocos2d::Color4F(1,1,1,0.4f));
    _waterBarBg->drawLine(br, tr, cocos2d::Color4F(1,1,1,0.4f));
    _waterBarBg->drawLine(tr, tl, cocos2d::Color4F(1,1,1,0.4f));
    _waterBarBg->drawLine(tl, bl, cocos2d::Color4F(1,1,1,0.4f));
    refreshHotbarOverlay();
}

void WaterCan::refreshHotbarOverlay() {
    if (!_iconSprite || !_waterBarNode || !_waterBarFill) return;
    auto cs = _iconSprite->getContentSize();
    float iconH = cs.height * _iconSprite->getScaleY();
    float marginY = 6.0f;
    auto pos = _iconSprite->getPosition();
    _waterBarNode->setPosition(cocos2d::Vec2(pos.x, pos.y + iconH/2 + marginY));

    auto &ws = Game::globalState();
    int desiredMax = waterCanMaxForLevel(level());
    if (ws.maxWater != desiredMax) {
        if (ws.water > desiredMax) {
            ws.water = desiredMax;
        }
        ws.maxWater = desiredMax;
    }
    float bw = _cellW;
    float bh = 8.0f;
    _waterBarFill->clear();
    float ratio = ws.maxWater > 0 ? (static_cast<float>(ws.water) / static_cast<float>(ws.maxWater)) : 0.f;
    ratio = std::max(0.f, std::min(1.f, ratio));
    float fillW = bw * ratio;
    cocos2d::Vec2 bl(-bw/2, 0), br(-bw/2 + fillW, 0), tr(-bw/2 + fillW, bh), tl(-bw/2, bh);
    cocos2d::Vec2 fillRect[4] = { bl, br, tr, tl };
    _waterBarFill->drawSolidPoly(fillRect, 4, cocos2d::Color4F(0.2f, 0.5f, 1.0f, 0.85f));
}

void WaterCan::detachHotbarOverlay() {
    if (_waterBarNode) {
        _waterBarNode->removeFromParent();
    }
    _waterBarNode = nullptr;
    _waterBarBg = nullptr;
    _waterBarFill = nullptr;
    _iconSprite = nullptr;
}

}
