/**
 * MineInteractor: 矿洞场景中与“空格键交互”相关的判定入口。
 * 负责：根据当前楼层与玩家世界坐标，判断这次空格键应该触发什么动作
 *      （下楼、回农场、回入口、打开电梯面板，或什么也不做）。
 * 协作对象：通过 MineMapController 提供的接口查询“玩家附近是否有楼梯/门/电梯”等，
 *          不直接访问地图内部数据；玩家位置通过传入的回调函数获取。
 */
#pragma once

#include "cocos2d.h"
#include "Controllers/Map/MineMapController.h"
#include <functional>

namespace Controllers {

class MineInteractor {
public:
    // 空格键在矿洞中可能触发的动作枚举；调用方根据返回值做后续处理
    enum class SpaceAction { None, Descend, ReturnToFarm, ReturnToEntrance, UseElevator };

    // 构造函数
    // 参数 map         : 当前矿洞地图控制器，用来查询楼层与附近环境
    // 参数 getPlayerPos: 一个“无参数，返回 Vec2”的函数对象，用来获取玩家当前位置
    // 说明：这里使用 std::function 而不是直接传坐标，是为了在每次按键时
    //       都能拿到“最新”的玩家位置，而不是构造时的一次性数值。
    MineInteractor(Controllers::MineMapController* map,
                    std::function<cocos2d::Vec2()> getPlayerPos)
    : _map(map), _getPlayerPos(std::move(getPlayerPos)) {}

    // 处理空格键交互：根据玩家所在楼层与位置返回应执行的动作
    // 注意：这里只做“规则判断 + 返回枚举”，不直接做场景切换或播放动画，
    //       方便调用方（例如控制器）统一编排后续流程。
    SpaceAction onSpacePressed();

private:
    // 矿洞地图控制器指针，仅作接口调用，不负责释放
    Controllers::MineMapController* _map = nullptr;
    // 获取玩家世界坐标的回调：调用时执行函数体，得到当前位置
    std::function<cocos2d::Vec2()> _getPlayerPos;
};

}
// namespace Controllers
