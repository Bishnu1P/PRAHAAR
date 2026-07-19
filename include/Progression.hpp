#pragma once
#include <functional>

// Tracks XP and level. Matches docs/design-doc.md:
//   xp_to_next_level(level) = 5 + (level * 3)
// Multiple level-ups from one big XP gain are handled correctly via
// the while loop (e.g. picking up a big orb that crosses 2 level
// thresholds at once triggers onLevelUp twice).
class Progression {
public:
    void addXP(float amount, const std::function<void()>& onLevelUp) {
        xp += amount;
        while (xp >= xpToNextLevel()) {
            xp -= xpToNextLevel();
            level++;
            if (onLevelUp) onLevelUp();
        }
    }

    float xpToNextLevel() const {
        return 5.f + level * 3.f;
    }

    int getLevel() const { return level; }
    float getXP() const { return xp; }

private:
    int level = 1;
    float xp = 0.f;
};
