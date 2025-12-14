#include "Controllers/TileSelector.h"
#include <cmath>

namespace {

struct TileCandidate {
    int c;
    int r;
};

int buildFanCandidates(int pc,
                       int pr,
                       const cocos2d::Vec2& dir,
                       const std::function<bool(int,int)>& inBounds,
                       TileCandidate* outCands) {
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
    if (dr != 0) {
        int fr = pr + dr;
        TileCandidate center{pc, fr};
        TileCandidate left{pc - 1, fr};
        TileCandidate right{pc + 1, fr};
        TileCandidate arr[3] = { center, left, right };
        for (int i = 0; i < 3; ++i) {
            if (!inBounds || inBounds(arr[i].c, arr[i].r)) {
                outCands[count++] = arr[i];
            }
        }
    } else if (dc != 0) {
        int fc = pc + dc;
        TileCandidate center{fc, pr};
        TileCandidate up{fc, pr + 1};
        TileCandidate down{fc, pr - 1};
        TileCandidate arr[3] = { center, up, down };
        for (int i = 0; i < 3; ++i) {
            if (!inBounds || inBounds(arr[i].c, arr[i].r)) {
                outCands[count++] = arr[i];
            }
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
    bool hasLastClick,
    const cocos2d::Vec2& lastClickWorldPos,
    const std::function<cocos2d::Vec2(int,int)>& tileToWorld)
{
    int pc = 0;
    int pr = 0;
    if (worldToTileIndex) {
        worldToTileIndex(playerPos, pc, pr);
    }
    cocos2d::Vec2 dir = lastDir;
    if (dir.lengthSquared() < 0.0001f) {
        dir = cocos2d::Vec2(0, -1);
    }
    TileCandidate cands[3];
    int count = buildFanCandidates(pc, pr, dir, inBounds, cands);
    if (count == 0) return { pc, pr };
    if (hasLastClick && tileToWorld) {
        cocos2d::Vec2 click = lastClickWorldPos;
        if (std::abs(dir.x) > std::abs(dir.y)) {
            click.y = 2.0f * playerPos.y - click.y;
        }
        float best = 1e9f;
        int bestIdx = 0;
        for (int i = 0; i < count; ++i) {
            cocos2d::Vec2 center = tileToWorld(cands[i].c, cands[i].r);
            float dx = center.x - click.x;
            float dy = center.y - click.y;
            float d2 = dx * dx + dy * dy;
            if (d2 < best) {
                best = d2;
                bestIdx = i;
            }
        }
        return { cands[bestIdx].c, cands[bestIdx].r };
    }
    return { cands[0].c, cands[0].r };
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
        worldToTileIndex(playerPos, pc, pr);
    }
    cocos2d::Vec2 dir = lastDir;
    if (dir.lengthSquared() < 0.0001f) {
        dir = cocos2d::Vec2(0, -1);
    }
    TileCandidate cands[3];
    int count = buildFanCandidates(pc, pr, dir, inBounds, cands);
    if (count == 0) return;
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
