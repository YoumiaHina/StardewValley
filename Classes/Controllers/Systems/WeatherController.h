/**
 * WeatherController：简单天气控制器。
 * - 职责：维护晴天/雨天状态，并将效果同步到地图（浇水、亮度）与玩家移动速度。
 * - 协作：通过 IMapController 接口改动瓦片状态；通过 PlayerController 接口改动移动速度；在 worldNode 上挂接暗化遮罩。
 */
#pragma once

#include "cocos2d.h"

namespace Controllers {

class IMapController;
class PlayerController;

enum class WeatherKind {
    Sunny = 0,
    Rainy = 1,
};

class WeatherController {
public:
    // 绑定地图/世界节点/玩家控制器；会立即按当前全局状态应用一次天气效果。
    WeatherController(IMapController* map, cocos2d::Node* worldNode, PlayerController* playerController);

    // 每帧更新：检测“跨天”并刷新天气，同时把效果同步到地图/玩家/亮度。
    void update(float dt);

    // 强制设置天气（同时写入全局状态并同步效果）。
    void setWeather(WeatherKind w);

    // 获取当前天气。
    WeatherKind weather() const { return _weather; }

private:
    void syncFromWorldState(bool forceApplyRainWatering);
    void ensureDimLayer();

private:
    IMapController* _map = nullptr;
    cocos2d::Node* _worldNode = nullptr;
    PlayerController* _playerController = nullptr;
    cocos2d::LayerColor* _dimLayer = nullptr;

    WeatherKind _weather = WeatherKind::Sunny;
    int _appliedSeasonIndex = -1;
    int _appliedDayOfSeason = -1;
};

} // namespace Controllers
