#pragma once

#include "cocos2d.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/Map/RoomMapController.h"

namespace Controllers {

class PlaceablePlacementBase {
public:
    static bool selectFarmCenter(IMapController* map,
                                 const cocos2d::Vec2& playerPos,
                                 const cocos2d::Vec2& lastDir,
                                 cocos2d::Vec2& outCenter);

    static bool selectRoomCenter(RoomMapController* room,
                                 const cocos2d::Vec2& playerPos,
                                 cocos2d::Vec2& outCenter);

    static bool selectOutdoorCenter(IMapController* map,
                                    const cocos2d::Vec2& playerPos,
                                    cocos2d::Vec2& outCenter);
};

inline bool PlaceablePlacementBase::selectFarmCenter(IMapController* map,
                                                     const cocos2d::Vec2& playerPos,
                                                     const cocos2d::Vec2& lastDir,
                                                     cocos2d::Vec2& outCenter) {
    if (!map) return false;
    if (!map->isFarm()) return false;
    auto tgt = map->targetTile(playerPos, lastDir);
    int tc = tgt.first;
    int tr = tgt.second;
    if (!map->inBounds(tc, tr)) return false;
    outCenter = map->tileToWorld(tc, tr);
    return true;
}

inline bool PlaceablePlacementBase::selectRoomCenter(RoomMapController* room,
                                                     const cocos2d::Vec2& playerPos,
                                                     cocos2d::Vec2& outCenter) {
    if (!room) return false;
    outCenter = playerPos;
    return true;
}

inline bool PlaceablePlacementBase::selectOutdoorCenter(IMapController* map,
                                                        const cocos2d::Vec2& playerPos,
                                                        cocos2d::Vec2& outCenter) {
    if (!map) return false;
    outCenter = playerPos;
    return true;
}

} // namespace Controllers

