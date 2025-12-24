#pragma once

#include "Game/NPC/NpcBase.h"

namespace Game {

// Robin NPC 数据（木匠/动物商贩）：
// - 只提供基础的名称与立绘路径。
// - 目前对所有礼物采用统一好感增量。
class Robin : public NpcBase {
 public:
  const char* name() const override;
  const char* texturePath() const override;
  int friendshipGainForGift(ItemType itemType) const override;
};

}  // namespace Game

