/**
 * FestivalController：节日控制器。
 * - 职责：根据全局日期判定是否为节日，并将结果同步到当前地图（图层显示/碰撞切换）。
 * - 职责边界：只负责“节日开关”的判定与通知，不包含具体地图解析细节。
 * - 协作对象：通过 IMapController::setFestivalActive 与各地图控制器协作。
 */
#pragma once

namespace Controllers {

class IMapController;

class FestivalController {
public:
    // 构造：绑定地图控制器，并立即按当前日期同步一次节日开关。
    explicit FestivalController(IMapController* map);

    // 每帧更新：检测跨天并在需要时重新同步节日开关。
    void update(float dt);

private:
    // 从全局状态读取并判定当天是否为节日。
    bool isFestivalToday() const;

    // 将当前节日开关写入地图控制器。
    void syncToMap(bool force);

private:
    IMapController* _map = nullptr;
    bool _festivalActive = false;
    int _appliedSeasonIndex = -1;
    int _appliedDayOfSeason = -1;
};

} // namespace Controllers

