#pragma once

#include <memory>
#include "Game/Tool/ToolBase.h"

namespace Game {

std::shared_ptr<ToolBase> makeTool(ToolKind kind);

}

