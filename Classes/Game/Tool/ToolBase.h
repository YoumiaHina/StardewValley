#pragma once

#include <string>
#include <functional>
#include "cocos2d.h"

namespace Controllers { class IMapController; class UIController; class CropSystem; }

namespace Game {

// 工具种类枚举：用来区分不同的工具类型。
// 使用 enum class 而不是传统 enum，能避免不同枚举互相隐式转换成 int。
enum class ToolKind { Axe, Hoe, Pickaxe, WaterCan, FishingRod, Sword, Scythe };

// 工具基类：所有具体工具（斧头、锄头、镐子等）都从这里继承。
// 职责：
// - 提供统一的接口：kind()/displayName()/use()/iconPath() 等；
// - 保存所有工具共有的状态（例如强化等级 level）。
// 协作对象（通过接口）：
// - Controllers::IMapController：负责地图坐标、环境系统等；
// - Controllers::CropSystem：负责农作物逻辑；
// - Controllers::UIController：负责 HUD、浮动文字、背包 UI 刷新。
class ToolBase {
public:
    // 获取当前工具等级（0=基础，1=铜，2=铁，3=金...）
    int level() const { return _level; }
    // 设置当前工具等级；外部一般通过升级系统统一修改。
    void setLevel(int lv) { _level = lv; }

    // 虚析构函数：保证通过基类指针 delete 派生类时能正确调用子类析构。
    virtual ~ToolBase() = default;
    // 纯虚函数：返回工具具体类型。子类必须实现。
    virtual ToolKind kind() const = 0;
    // 纯虚函数：返回工具在 UI 中显示的名称。
    virtual std::string displayName() const = 0;
    // 纯虚函数：使用工具时被调用，返回一段提示文本（例如 "Chop!"）。
    // 参数说明：
    // - map：地图控制接口，提供坐标转换、环境系统访问等功能；
    // - crop：农作物系统，某些工具需要操作作物（如 Hoe、WaterCan）；
    // - getPlayerPos：函数对象，调用它可以获取当前玩家位置；
    // - getLastDir：函数对象，调用它可以获取玩家最后朝向；
    // - ui：UI 控制器，用于弹出文字、刷新 HUD/热键栏等。
    // std::function 相当于一个“可以保存任意可调用对象”的函数指针封装，
    // 既可以保存普通函数指针，也可以保存 lambda。
    virtual std::string use(Controllers::IMapController* map,
                            Controllers::CropSystem* crop,
                            std::function<cocos2d::Vec2()> getPlayerPos,
                            std::function<cocos2d::Vec2()> getLastDir,
                            Controllers::UIController* ui) = 0;

    // 返回在 UI 中使用的图标路径。基类默认返回空字符串，子类可以覆盖。
    virtual std::string iconPath() const { return std::string(); }

    // 以下三个接口用于在背包热键栏的格子上绘制额外信息（例如浇水壶水量条）：
    // attachHotbarOverlay：当工具被放入某个格子时调用，用来创建并挂载额外节点；
    // refreshHotbarOverlay：状态变化时调用，用来更新显示（例如水量比例）；
    // detachHotbarOverlay：工具移出该格子时调用，用来移除节点，避免内存泄漏。
    virtual void attachHotbarOverlay(cocos2d::Sprite* /*iconSprite*/, float /*cellW*/, float /*cellH*/) {}
    virtual void refreshHotbarOverlay() {}
    virtual void detachHotbarOverlay() {}

protected:
    // 工具等级，默认 0。由 ToolUpgradeSystem 或存档加载逻辑进行调整。
    int _level = 0;
};

}
