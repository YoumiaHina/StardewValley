#pragma once

#include "Game/NPC/NpcBase.h"

namespace Game {

// Pierre NPC 数据（杂货店老板）：
// - 定义名称与立绘路径。
// - 当前赠礼好感规则较为简单：所有物品统一加固定值。
class Pierre : public NpcBase {
 public:
  const char* name() const override;
  const char* texturePath() const override;
  int friendshipGainForGift(ItemType itemType) const override;
};

}  // namespace Game

