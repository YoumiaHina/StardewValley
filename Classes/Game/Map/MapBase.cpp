// 地图基础实现：
// - 负责 TMX 地图文件的加载与节点挂载
// - 提供通用的瓦片/世界坐标转换工具
// - 实现多边形与矩形碰撞、邻近与几何中心计算
#include "MapBase.h"

using namespace cocos2d;

namespace Game {

bool MapBase::initWithFile(const std::string& tmxFile) {
    if (!Node::init()) return false;
    _tmx = TMXTiledMap::create(tmxFile);
    if (!_tmx) return false;
    this->addChild(_tmx);
    return true;
}

Size MapBase::getMapSize() const {
    return _tmx ? _tmx->getMapSize() : Size::ZERO;
}

Size MapBase::getTileSize() const {
    return _tmx ? _tmx->getTileSize() : Size(GameConfig::TILE_SIZE, GameConfig::TILE_SIZE);
}

const Size& MapBase::getContentSize() const {
    if (_tmx) return _tmx->getContentSize();
    static Size zero;
    zero = Size::ZERO;
    return zero;
}

Vec2 MapBase::tileToWorld(int c, int r) const {
    float s = _tmx ? _tmx->getTileSize().width : (float)GameConfig::TILE_SIZE;
    return Vec2(c * s + s * 0.5f, r * s + s * 0.5f);
}

void MapBase::worldToTileIndex(const Vec2& p, int& c, int& r) const {
    float s = _tmx ? _tmx->getTileSize().width : (float)GameConfig::TILE_SIZE;
    c = static_cast<int>(p.x / s);
    r = static_cast<int>(p.y / s);
}

bool MapBase::collidesAt(const Vec2& p, float radius,
                         const std::vector<Rect>& rects,
                         const std::vector<std::vector<Vec2>>& polys) {
    float r2 = radius * radius;
    for (const auto& r : rects) {
        float cx = std::max(r.getMinX(), std::min(p.x, r.getMaxX()));
        float cy = std::max(r.getMinY(), std::min(p.y, r.getMaxY()));
        float dx = p.x - cx; float dy = p.y - cy;
        if (dx*dx + dy*dy <= r2) return true;
    }
    for (const auto& poly : polys) {
        if (poly.size() < 3) continue;
        bool inside = false; size_t j = poly.size() - 1;
        for (size_t i = 0; i < poly.size(); ++i) {
            if (((poly[i].y > p.y) != (poly[j].y > p.y)) &&
                (p.x < (poly[j].x - poly[i].x) * (p.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x)) {
                inside = !inside;
            }
            j = i;
        }
        if (inside) return true;
        j = poly.size() - 1;
        for (size_t i = 0; i < poly.size(); ++i) {
            Vec2 p1 = poly[j]; Vec2 p2 = poly[i]; Vec2 d = p2 - p1;
            if (d.lengthSquared() > 0) {
                float t = (p - p1).dot(d) / d.lengthSquared(); t = std::max(0.0f, std::min(1.0f, t));
                Vec2 close = p1 + d * t; if (p.distanceSquared(close) <= r2) return true;
            }
            j = i;
        }
    }
    return false;
}

Vec2 MapBase::centerFromRectsPoints(const std::vector<Rect>& rects,
                                    const std::vector<Vec2>& points) {
    if (!rects.empty()) { const auto& r = rects.front(); return Vec2(r.getMidX(), r.getMidY()); }
    if (!points.empty()) return points.front();
    return Vec2::ZERO;
}

bool MapBase::nearRectOrPoints(const Vec2& p,
                               const std::vector<Rect>& rects,
                               const std::vector<Vec2>& points,
                               float radiusSquared) {
    for (const auto& r : rects) { if (r.containsPoint(p)) return true; }
    for (const auto& pt : points) { if (p.distanceSquared(pt) <= radiusSquared) return true; }
    return false;
}

void MapBase::parseWalls(TMXTiledMap* tmx,
                         std::vector<Rect>& outRects,
                         std::vector<std::vector<Vec2>>& outPolys,
                         DrawNode* debugTarget) {
    outRects.clear();
    outPolys.clear();
    if (!tmx) return;
    auto group = tmx->getObjectGroup("Wall");
    if (!group) group = tmx->getObjectGroup("wall");
    if (!group) group = tmx->getObjectGroup("Collision");
    if (!group) group = tmx->getObjectGroup("collision");
    if (!group) return;
    auto objects = group->getObjects();
    for (auto &val : objects) {
        auto dict = val.asValueMap();
        float x = dict.at("x").asFloat();
        float y = dict.at("y").asFloat();
        if (dict.find("points") != dict.end() || dict.find("polyline") != dict.end() || dict.find("polygon") != dict.end()) {
            std::vector<Vec2> pts; ValueVector arr;
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
                outPolys.push_back(pts);
                if (debugTarget) {
                    debugTarget->drawPoly(pts.data(), (unsigned int)pts.size(), true, Color4F(1, 0, 0, 0.5f));
                    debugTarget->drawSolidPoly(pts.data(), (unsigned int)pts.size(), Color4F(1, 0, 0, 0.2f));
                }
            }
        } else if (dict.find("width") != dict.end() && dict.find("height") != dict.end()) {
            float w = dict.at("width").asFloat(); float h = dict.at("height").asFloat();
            Rect r(x, y, w, h);
            outRects.push_back(r);
            if (debugTarget) {
                debugTarget->drawRect(r.origin, r.origin + r.size, Color4F(1, 0, 0, 0.5f));
                debugTarget->drawSolidRect(r.origin, r.origin + r.size, Color4F(1, 0, 0, 0.2f));
            }
        }
    }
}

} // namespace Game
