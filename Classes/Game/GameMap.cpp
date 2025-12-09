#include "GameMap.h"

namespace Game {

GameMap* GameMap::create(const std::string& tmxFile) {
    GameMap* ret = new (std::nothrow) GameMap();
    if (ret && ret->initWithFile(tmxFile)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool GameMap::initWithFile(const std::string& tmxFile) {
    if (!Node::init()) return false;

    _tmx = cocos2d::TMXTiledMap::create(tmxFile);
    if (!_tmx) {
        cocos2d::log("Failed to load TMX file: %s", tmxFile.c_str());
        return false;
    }

    this->addChild(_tmx);
    
    setupLayerOrder();
    parseWalls();
    parseWater();

    return true;
}

void GameMap::setupLayerOrder() {
    if (!_tmx) return;
    _bgLayer = _tmx->getLayer("Background");
    _houseBodyLayer = _tmx->getLayer("HouseBody");
    _houseTopLayer = _tmx->getLayer("HouseTop");

    if (_bgLayer) _tmx->reorderChild(_bgLayer, 0);
    if (_houseBodyLayer) _tmx->reorderChild(_houseBodyLayer, 10);
    if (_houseTopLayer) _tmx->reorderChild(_houseTopLayer, 30);
}

void GameMap::parseWalls() {
    _wallRects.clear();
    _wallPolygons.clear();
    if (!_tmx) return;
    
    // Debug node for walls
    if (_debugNode) {
        _debugNode->removeFromParent();
        _debugNode = nullptr;
    }
    _debugNode = cocos2d::DrawNode::create();
    _tmx->addChild(_debugNode, 999); // High Z to be visible

    auto group = _tmx->getObjectGroup("Wall");
    if (!group) return;
    auto objects = group->getObjects();
    
    cocos2d::log("Parsing Wall Objects: Found %d objects", (int)objects.size());

    for (auto &val : objects) {
        auto dict = val.asValueMap();
        float x = dict.at("x").asFloat();
        float y = dict.at("y").asFloat(); 
        
        if (dict.find("points") != dict.end() || dict.find("polyline") != dict.end() || dict.find("polygon") != dict.end()) {
             // Polygon / Polyline
             std::vector<cocos2d::Vec2> pts;
             cocos2d::ValueVector arr;
             if (dict.find("points") != dict.end()) arr = dict.at("points").asValueVector();
             else if (dict.find("polygon") != dict.end()) arr = dict.at("polygon").asValueVector();
             else if (dict.find("polyline") != dict.end()) arr = dict.at("polyline").asValueVector();

             for (auto &pv : arr) {
                 auto pmap = pv.asValueMap();
                 float px = pmap.at("x").asFloat();
                 float py = pmap.at("y").asFloat();
                 // TMX polygon points are relative offsets.
                 // TMX Y is down, Cocos Y is up. 
                 // So +py in TMX means moving DOWN. In Cocos, that means SUBTRACTING from Y.
                 float finalX = x + px;
                 float finalY = y - py; 
                 pts.emplace_back(finalX, finalY);
             }
             
             if (!pts.empty()) {
                 _wallPolygons.push_back(pts);
                 // Draw debug poly (Red)
                 _debugNode->drawPoly(pts.data(), (unsigned int)pts.size(), true, cocos2d::Color4F(1, 0, 0, 0.5f));
                 _debugNode->drawSolidPoly(pts.data(), (unsigned int)pts.size(), cocos2d::Color4F(1, 0, 0, 0.2f));
                 cocos2d::log("Added Polygon with %d points", (int)pts.size());
             }
        } else if (dict.find("width") != dict.end() && dict.find("height") != dict.end()) {
            // Rectangle
            float w = dict.at("width").asFloat();
            float h = dict.at("height").asFloat();
            // cocos2d-x TMX parser usually returns 'y' as the bottom-left corner of the rect
            cocos2d::Rect r(x, y, w, h);
            _wallRects.push_back(r);
            
            // Draw debug rect (Red)
            _debugNode->drawRect(r.origin, r.origin + r.size, cocos2d::Color4F(1, 0, 0, 0.5f));
            _debugNode->drawSolidRect(r.origin, r.origin + r.size, cocos2d::Color4F(1, 0, 0, 0.2f));
            cocos2d::log("Added Rect: x=%.2f, y=%.2f, w=%.2f, h=%.2f", x, y, w, h);
        }
    }
}

bool GameMap::collides(const cocos2d::Vec2& p, float radius) const {
    for (const auto& r : _wallRects) {
        float cx = std::max(r.getMinX(), std::min(p.x, r.getMaxX()));
        float cy = std::max(r.getMinY(), std::min(p.y, r.getMaxY()));
        float dx = p.x - cx;
        float dy = p.y - cy;
        if (dx*dx + dy*dy <= radius*radius) return true;
    }
    for (const auto& poly : _wallPolygons) {
        if (poly.size() < 3) continue;
        bool inside = false;
        size_t j = poly.size() - 1;
        for (size_t i = 0; i < poly.size(); i++) {
            if ( ((poly[i].y > p.y) != (poly[j].y > p.y)) &&
                 (p.x < (poly[j].x - poly[i].x) * (p.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x) ) {
               inside = !inside;
            }
            j = i;
        }
        if (inside) return true;
        j = poly.size() - 1;
        float r2 = radius * radius;
        for (size_t i = 0; i < poly.size(); i++) {
            cocos2d::Vec2 p1 = poly[j];
            cocos2d::Vec2 p2 = poly[i];
            cocos2d::Vec2 d = p2 - p1;
            if (d.lengthSquared() > 0) {
                float t = (p - p1).dot(d) / d.lengthSquared();
                t = std::max(0.0f, std::min(1.0f, t));
                cocos2d::Vec2 closest = p1 + d * t;
                if (p.distanceSquared(closest) <= r2) return true;
            }
            j = i;
        }
    }
    for (const auto& r : _waterRects) {
        float cx = std::max(r.getMinX(), std::min(p.x, r.getMaxX()));
        float cy = std::max(r.getMinY(), std::min(p.y, r.getMaxY()));
        float dx = p.x - cx;
        float dy = p.y - cy;
        if (dx*dx + dy*dy <= radius*radius) return true;
    }
    for (const auto& poly : _waterPolygons) {
        if (poly.size() < 3) continue;
        bool inside = false;
        size_t j = poly.size() - 1;
        for (size_t i = 0; i < poly.size(); i++) {
            if ( ((poly[i].y > p.y) != (poly[j].y > p.y)) &&
                 (p.x < (poly[j].x - poly[i].x) * (p.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x) ) {
               inside = !inside;
            }
            j = i;
        }
        if (inside) return true;
        j = poly.size() - 1;
        float r2 = radius * radius;
        for (size_t i = 0; i < poly.size(); i++) {
            cocos2d::Vec2 p1 = poly[j];
            cocos2d::Vec2 p2 = poly[i];
            cocos2d::Vec2 d = p2 - p1;
            if (d.lengthSquared() > 0) {
                float t = (p - p1).dot(d) / d.lengthSquared();
                t = std::max(0.0f, std::min(1.0f, t));
                cocos2d::Vec2 closest = p1 + d * t;
                if (p.distanceSquared(closest) <= r2) return true;
            }
            j = i;
        }
    }
    return false;
}

cocos2d::Size GameMap::getMapSize() const {
    if (_tmx) return _tmx->getMapSize();
    return cocos2d::Size::ZERO;
}

cocos2d::Size GameMap::getTileSize() const {
    if (_tmx) return _tmx->getTileSize();
    return cocos2d::Size(GameConfig::TILE_SIZE, GameConfig::TILE_SIZE);
}

const cocos2d::Size& GameMap::getContentSize() const {
    if (_tmx) return _tmx->getContentSize();
    return cocos2d::Size::ZERO;
}

cocos2d::Vec2 GameMap::tileToWorld(int c, int r) const {
    float s = _tmx ? _tmx->getTileSize().width : (float)GameConfig::TILE_SIZE;
    // TMX usually starts from top-left, but our logic in GameScene assumed bottom-left for _tiles?
    // In GameScene::tileToWorld, it was:
    // if (_tmx) return Vec2(c * s + s * 0.5f, r * s + s * 0.5f);
    // This assumes (0,0) is bottom-left of the TMX node.
    // TMX node anchor is (0,0), so its local (0,0) is bottom-left.
    // So this is correct relative to the map node.
    return cocos2d::Vec2(c * s + s * 0.5f, r * s + s * 0.5f);
}

void GameMap::worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const {
    // p is in local coordinates of GameMap (which matches TMX node coords)
    float s = _tmx ? _tmx->getTileSize().width : (float)GameConfig::TILE_SIZE;
    c = static_cast<int>(p.x / s);
    r = static_cast<int>(p.y / s);
}

void GameMap::parseWater() {
    _waterRects.clear();
    _waterPolygons.clear();
    if (!_tmx) return;
    if (_waterDebugNode) { _waterDebugNode->removeFromParent(); _waterDebugNode = nullptr; }
    _waterDebugNode = cocos2d::DrawNode::create();
    _tmx->addChild(_waterDebugNode, 998);
    auto group = _tmx->getObjectGroup("Water");
    if (!group) group = _tmx->getObjectGroup("water");
    if (!group) return;
    auto objects = group->getObjects();
    for (auto &val : objects) {
        auto dict = val.asValueMap();
        float x = dict.at("x").asFloat();
        float y = dict.at("y").asFloat();
        if (dict.find("points") != dict.end() || dict.find("polyline") != dict.end() || dict.find("polygon") != dict.end()) {
            std::vector<cocos2d::Vec2> pts;
            cocos2d::ValueVector arr;
            if (dict.find("points") != dict.end()) arr = dict.at("points").asValueVector();
            else if (dict.find("polygon") != dict.end()) arr = dict.at("polygon").asValueVector();
            else if (dict.find("polyline") != dict.end()) arr = dict.at("polyline").asValueVector();
            for (auto &pv : arr) {
                auto pmap = pv.asValueMap();
                float px = pmap.at("x").asFloat();
                float py = pmap.at("y").asFloat();
                float finalX = x + px;
                float finalY = y - py;
                pts.emplace_back(finalX, finalY);
            }
            if (!pts.empty()) {
                _waterPolygons.push_back(pts);
                _waterDebugNode->drawPoly(pts.data(), (unsigned int)pts.size(), true, cocos2d::Color4F(0, 0, 1, 0.4f));
                _waterDebugNode->drawSolidPoly(pts.data(), (unsigned int)pts.size(), cocos2d::Color4F(0, 0, 1, 0.2f));
            }
        } else if (dict.find("width") != dict.end() && dict.find("height") != dict.end()) {
            float w = dict.at("width").asFloat();
            float h = dict.at("height").asFloat();
            cocos2d::Rect r(x, y, w, h);
            _waterRects.push_back(r);
            _waterDebugNode->drawRect(r.origin, r.origin + r.size, cocos2d::Color4F(0, 0, 1, 0.4f));
            _waterDebugNode->drawSolidRect(r.origin, r.origin + r.size, cocos2d::Color4F(0, 0, 1, 0.2f));
        }
    }
}

bool GameMap::nearWater(const cocos2d::Vec2& p, float radius) const {
    for (const auto& r : _waterRects) {
        float cx = std::max(r.getMinX(), std::min(p.x, r.getMaxX()));
        float cy = std::max(r.getMinY(), std::min(p.y, r.getMaxY()));
        float dx = p.x - cx;
        float dy = p.y - cy;
        if (dx*dx + dy*dy <= radius*radius) return true;
    }
    for (const auto& poly : _waterPolygons) {
        if (poly.size() < 2) continue;
        size_t j = poly.size() - 1;
        float r2 = radius * radius;
        for (size_t i = 0; i < poly.size(); i++) {
            cocos2d::Vec2 p1 = poly[j];
            cocos2d::Vec2 p2 = poly[i];
            cocos2d::Vec2 d = p2 - p1;
            if (d.lengthSquared() > 0) {
                float t = (p - p1).dot(d) / d.lengthSquared();
                t = std::max(0.0f, std::min(1.0f, t));
                cocos2d::Vec2 closest = p1 + d * t;
                if (p.distanceSquared(closest) <= r2) return true;
            }
            j = i;
        }
    }
    return false;
}

} // namespace Game