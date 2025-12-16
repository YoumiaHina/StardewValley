#pragma once

#include "Game/NPC/NpcBase.h"

namespace Game {

class Pierre : public NpcBase {
 public:
  const char* name() const override;
  const char* texturePath() const override;
  int friendshipGainForGift(ItemType itemType) const override;
};

}  // namespace Game

