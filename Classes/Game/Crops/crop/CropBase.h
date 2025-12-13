// 种下状态的作物行为接口：
// - 仅定义行为判定与状态变更；静态数据见 Game/Crop.h / CropDefs
// - 实现类（如 ParsnipCrop/CornCrop）以具体规则覆盖以下方法
// - 注意：所有方法不访问除传入 Crop 外的其他状态；模块通过接口协作
#pragma once

#include "Game/Crop.h"

namespace Game {

class CropBase {
public:
    virtual ~CropBase() = default;
    // 是否允许“加速一次”（用于作弊/道具触发）
    virtual bool canAccelerate(const Game::Crop& cp) const = 0;
    // 当前是否可收获（一般为成熟阶段或倒数阶段的特殊作物）
    virtual bool canHarvest(const Game::Crop& cp) const = 0;
    // 收获时的状态变更；remove=true 表示从地图移除该作物
    virtual void onHarvest(Game::Crop& cp, bool& remove) const = 0;
    // 每日回生长逻辑（如蓝莓/玉米等可回生作物），返回是否已处理并阻止常规推进
    virtual bool onDailyRegrow(Game::Crop& cp) const = 0;
    // 执行一次加速（阶段+1 或从最终回退至倒数阶段等），返回是否成功
    virtual bool accelerate(Game::Crop& cp) const = 0;
    // 收获是否产生成熟产物（用于背包添加/掉落），如“倒数阶段可采摘”
    virtual bool yieldsOnHarvest(const Game::Crop& cp) const = 0;
};

}
