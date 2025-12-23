#pragma once

#include <memory>
#include "Game/Tool/ToolBase.h"

namespace Game {

// 工具工厂函数：根据枚举 ToolKind 创建对应具体工具对象。
// 返回值使用 std::shared_ptr 管理生命周期，方便在背包等容器中共享持有。
std::shared_ptr<ToolBase> makeTool(ToolKind kind);

}
