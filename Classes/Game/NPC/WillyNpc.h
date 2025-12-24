#pragma once

#include "Game/NPC/NpcBase.h"

namespace Game {

// Willy NPC 数据（渔夫）：
// - 名称与立绘路径用于渔夫 NPC 的精灵与对话。
// - 赠礼好感规则偏向各种鱼类与部分农作物。
class Willy : public NpcBase {
 public:
  const char* name() const override;
  const char* texturePath() const override;
  int friendshipGainForGift(ItemType itemType) const override;
};

}  // namespace Game

