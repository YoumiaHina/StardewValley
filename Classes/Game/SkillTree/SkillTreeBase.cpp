#include "Game/SkillTree/SkillTreeBase.h"

#include <algorithm>
#include <limits>

namespace Game {

int SkillTreeBase::xpForNextLevel(int currentLevel) const {
    if (currentLevel < 0) currentLevel = 0;
    return 100 + currentLevel * 50;
}

int SkillTreeBase::totalXpRequiredForLevel(int level) const {
    if (level <= 0) return 0;
    int cappedLevel = std::min(level, maxLevel());
    long long sum = 0;
    for (int lv = 0; lv < cappedLevel; ++lv) {
        sum += xpForNextLevel(lv);
    }
    if (sum < 0) return 0;
    if (sum > static_cast<long long>(std::numeric_limits<int>::max())) {
        return std::numeric_limits<int>::max();
    }
    return static_cast<int>(sum);
}

int SkillTreeBase::levelFromTotalXp(int totalXp) const {
    if (totalXp <= 0) return 0;
    int lvl = 0;
    int cappedMax = std::max(0, maxLevel());
    long long remaining = totalXp;
    while (lvl < cappedMax) {
        int need = xpForNextLevel(lvl);
        if (need <= 0) break;
        if (remaining < need) break;
        remaining -= need;
        lvl += 1;
    }
    return lvl;
}

int SkillTreeBase::xpToNextLevelFromTotalXp(int totalXp) const {
    int cappedMax = std::max(0, maxLevel());
    int curXp = std::max(0, totalXp);
    int lvl = levelFromTotalXp(curXp);
    if (lvl >= cappedMax) return 0;
    int nextLevelTotal = totalXpRequiredForLevel(lvl + 1);
    return std::max(0, nextLevelTotal - curXp);
}

const SkillTreeBase& skillTreeByType(SkillTreeType t) {
    switch (t) {
        case SkillTreeType::Farming:
            return farmingSkillTree();
        case SkillTreeType::AnimalHusbandry:
            return animalHusbandrySkillTree();
        case SkillTreeType::Forestry:
            return forestrySkillTree();
        case SkillTreeType::Fishing:
            return fishingSkillTree();
        case SkillTreeType::Mining:
            return miningSkillTree();
        case SkillTreeType::Combat:
            return combatSkillTree();
    }
    return farmingSkillTree();
}

}
