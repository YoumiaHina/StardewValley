#include "Controllers/Interact/TileSelector.h"
#include "Game/GameConfig.h"
#include <cmath>

namespace {

struct TileCandidate {
    int c;
    int r;
};

int buildFront3x3Candidates(int pc,
                            int pr,
                            const cocos2d::Vec2& dir,
                            const std::function<bool(int,int)>& inBounds,
                            TileCandidate* outCands,
                            int maxOut) {
    int dc = 0;
    int dr = 0;
    if (std::abs(dir.x) > std::abs(dir.y)) {
        dc = (dir.x > 0.1f) ? 1 : ((dir.x < -0.1f) ? -1 : 0);
    } else {
        dr = (dir.y > 0.1f) ? 1 : ((dir.y < -0.1f) ? -1 : 0);
    }
    if (dc == 0 && dr == 0) {
        dr = -1;
    }

    int count = 0;
    auto pushUnique = [&](int c, int r) {
        if (count >= maxOut) return;
        if (inBounds && !inBounds(c, r)) return;
        for (int i = 0; i < count; ++i) {
            if (outCands[i].c == c && outCands[i].r == r) return;
        }
        outCands[count++] = TileCandidate{c, r};
    };

    pushUnique(pc, pr);

    int fc = pc + dc;
    int fr = pr + dr;
    for (int rr = fr - 1; rr <= fr + 1; ++rr) {
        for (int cc = fc - 1; cc <= fc + 1; ++cc) {
            pushUnique(cc, rr);
        }
    }

    return count;
}

} // anonymous

namespace Controllers {

std::pair<int,int> TileSelector::selectForwardTile(
    const cocos2d::Vec2& playerPos,
    const cocos2d::Vec2& lastDir,
    const std::function<void(const cocos2d::Vec2&, int&, int&)>& worldToTileIndex,
    const std::function<bool(int,int)>& inBounds,
    float tileSize,
    bool hasLastClick,
    const cocos2d::Vec2& lastClickWorldPos,
    const std::function<cocos2d::Vec2(int,int)>& tileToWorld)
{
    int pc = 0;
    int pr = 0;
    if (worldToTileIndex) {
        cocos2d::Vec2 basisPos = playerPos;
        if (tileSize > 0.0f) {
            basisPos = playerPos + cocos2d::Vec2(0, -tileSize * 0.75f);
        }
        worldToTileIndex(basisPos, pc, pr);
    }
    cocos2d::Vec2 dir = lastDir;
    if (dir.lengthSquared() < 0.0001f) {
        dir = cocos2d::Vec2(0, -1);
    }
    TileCandidate cands[10];
    int count = buildFront3x3Candidates(pc, pr, dir, inBounds, cands, 10);
    if (count <= 0) return { pc, pr };

    if (hasLastClick && tileToWorld && tileSize > 0.0f) {
        cocos2d::Vec2 click = lastClickWorldPos;
        float half = tileSize * 0.5f;
        for (int i = 0; i < count; ++i) {
            cocos2d::Vec2 center = tileToWorld(cands[i].c, cands[i].r);
            cocos2d::Rect rect(center.x - half, center.y - half, tileSize, tileSize);
            if (rect.containsPoint(click)) {
                return { cands[i].c, cands[i].r };
            }
        }
        return { -1, -1 };
    }

    int dc = 0;
    int dr = 0;
    if (std::abs(dir.x) > std::abs(dir.y)) {
        dc = (dir.x > 0.1f) ? 1 : ((dir.x < -0.1f) ? -1 : 0);
    } else {
        dr = (dir.y > 0.1f) ? 1 : ((dir.y < -0.1f) ? -1 : 0);
    }
    if (dc == 0 && dr == 0) {
        dr = -1;
    }

    int fc = pc + dc;
    int fr = pr + dr;
    if (!inBounds || inBounds(fc, fr)) {
        return { fc, fr };
    }
    return { pc, pr };
}

void TileSelector::drawFanCursor(
    cocos2d::DrawNode* cursor,
    const cocos2d::Vec2& playerPos,
    const cocos2d::Vec2& lastDir,
    const std::function<void(const cocos2d::Vec2&, int&, int&)>& worldToTileIndex,
    const std::function<bool(int,int)>& inBounds,
    const std::function<cocos2d::Vec2(int,int)>& tileToWorld,
    float tileSize)
{
    if (!cursor) return;
    cursor->clear();
    int pc = 0;
    int pr = 0;
    if (worldToTileIndex) {
        cocos2d::Vec2 basisPos = playerPos;
        if (tileSize > 0.0f) {
            basisPos = playerPos + cocos2d::Vec2(0, -tileSize * 0.75f);
        }
        worldToTileIndex(basisPos, pc, pr);
    }
    cocos2d::Vec2 dir = lastDir;
    if (dir.lengthSquared() < 0.0001f) {
        dir = cocos2d::Vec2(0, -1);
    }
    TileCandidate cands[10];
    int count = buildFront3x3Candidates(pc, pr, dir, inBounds, cands, 10);
    if (count <= 0) return;
    float s = tileSize;
    for (int i = 0; i < count; ++i) {
        int c = cands[i].c;
        int r = cands[i].r;
        cocos2d::Vec2 center = tileToWorld(c, r);
        cocos2d::Vec2 a(center.x - s * 0.5f, center.y - s * 0.5f);
        cocos2d::Vec2 b(center.x + s * 0.5f, center.y - s * 0.5f);
        cocos2d::Vec2 d(center.x - s * 0.5f, center.y + s * 0.5f);
        cocos2d::Vec2 e(center.x + s * 0.5f, center.y + s * 0.5f);
        cursor->drawLine(a, b, cocos2d::Color4F(1.f, 0.9f, 0.2f, 1.f));
        cursor->drawLine(b, e, cocos2d::Color4F(1.f, 0.9f, 0.2f, 1.f));
        cursor->drawLine(e, d, cocos2d::Color4F(1.f, 0.9f, 0.2f, 1.f));
        cursor->drawLine(d, a, cocos2d::Color4F(1.f, 0.9f, 0.2f, 1.f));
    }
}

} // namespace Controllers
