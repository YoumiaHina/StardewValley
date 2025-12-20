#include "Game/SkillTree/SkillTreeBase.h"

#include <algorithm>
#include <limits>

namespace Game {

static int clampInt(int v, int lo, int hi) {
    if (hi < lo) {
        int tmp = lo;
        lo = hi;
        hi = tmp;
    }
    return std::max(lo, std::min(v, hi));
}

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

void SkillTreeBase::syncProgress(SkillProgress& progress) const {
    if (progress.totalXp < 0) progress.totalXp = 0;
    int newLevel = levelFromTotalXp(progress.totalXp);
    int oldLevel = clampInt(progress.level, 0, std::max(0, maxLevel()));
    progress.level = newLevel;
    int gained = newLevel - oldLevel;
    if (gained > 0) {
        progress.unspentPoints += gained * pointsPerLevel();
        if (progress.unspentPoints < 0) progress.unspentPoints = 0;
    }
}

void SkillTreeBase::addXp(SkillProgress& progress, int deltaXp, int* outLevelsGained) const {
    int beforeLevel = clampInt(progress.level, 0, std::max(0, maxLevel()));
    long long nextXp = static_cast<long long>(progress.totalXp) + deltaXp;
    if (nextXp < 0) nextXp = 0;
    if (nextXp > static_cast<long long>(std::numeric_limits<int>::max())) {
        nextXp = std::numeric_limits<int>::max();
    }
    progress.totalXp = static_cast<int>(nextXp);
    syncProgress(progress);
    if (outLevelsGained) {
        *outLevelsGained = std::max(0, progress.level - beforeLevel);
    }
}

int SkillTreeBase::xpToNextLevel(const SkillProgress& progress) const {
    int cappedMax = std::max(0, maxLevel());
    int lvl = clampInt(progress.level, 0, cappedMax);
    if (lvl >= cappedMax) return 0;
    int nextLevelTotal = totalXpRequiredForLevel(lvl + 1);
    int curXp = std::max(0, progress.totalXp);
    return std::max(0, nextLevelTotal - curXp);
}

const SkillNode* SkillTreeBase::findNode(int nodeId) const {
    for (const auto& n : nodes()) {
        if (n.id == nodeId) return &n;
    }
    return nullptr;
}

bool SkillTreeBase::isNodeUnlocked(const SkillProgress& progress, int nodeId) const {
    return progress.unlockedNodeIds.find(nodeId) != progress.unlockedNodeIds.end();
}

bool SkillTreeBase::canUnlockNode(const SkillProgress& progress, int nodeId) const {
    const SkillNode* node = findNode(nodeId);
    if (!node) return false;
    if (isNodeUnlocked(progress, nodeId)) return false;
    if (progress.unspentPoints <= 0) return false;
    if (progress.level < node->requiredLevel) return false;
    return true;
}

bool SkillTreeBase::unlockNode(SkillProgress& progress, int nodeId) const {
    if (!canUnlockNode(progress, nodeId)) return false;
    progress.unspentPoints -= 1;
    if (progress.unspentPoints < 0) progress.unspentPoints = 0;
    progress.unlockedNodeIds.insert(nodeId);
    return true;
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
    }
    return farmingSkillTree();
}

}

