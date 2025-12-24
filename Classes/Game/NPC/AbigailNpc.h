#pragma once

#include "Game/NPC/NpcBase.h"

namespace Game {

// Abigail NPC 数据：
// - 提供名称、默认立绘路径。
// - 根据赠礼物品类型计算好感增益（更喜欢草莓和各种鱼类）。
class Abigail : public NpcBase {
 public:
  const char* name() const override;
  const char* texturePath() const override;
  int friendshipGainForGift(ItemType itemType) const override;
};

}  // namespace Game

