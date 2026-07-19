#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>

// Manages the 3 player-triggered powerups:
//   J - Burst: instant AoE damage around the player
//   K - Big Bullets: temporarily larger/stronger projectiles
//   L - Rapid Fire: temporarily faster weapon fire rate
// Each has its own cooldown (must "recharge" before reuse), matching
// the idea from the project proposal.
class PowerupSystem {
public:
    // Call once per fixed update. Returns true on the exact frame the
    // Burst powerup triggers (Game uses this to apply the AoE damage).
    bool update(float dt) {
        tickCooldown(burst, dt);
        tickCooldown(bigBullets, dt);
        tickCooldown(fireRate, dt);

        bool burstTriggeredNow = false;

        if (justPressed(sf::Keyboard::J, burstKeyWasDown) && burst.cooldownRemaining <= 0.f) {
            burst.cooldownRemaining = burst.cooldownDuration;
            burstTriggeredNow = true;
        }
        if (justPressed(sf::Keyboard::K, bigBulletsKeyWasDown) && bigBullets.cooldownRemaining <= 0.f) {
            bigBullets.cooldownRemaining = bigBullets.cooldownDuration;
            bigBullets.effectTimeRemaining = bigBullets.effectDuration;
        }
        if (justPressed(sf::Keyboard::L, fireRateKeyWasDown) && fireRate.cooldownRemaining <= 0.f) {
            fireRate.cooldownRemaining = fireRate.cooldownDuration;
            fireRate.effectTimeRemaining = fireRate.effectDuration;
        }

        return burstTriggeredNow;
    }

    bool isBigBulletsActive() const { return bigBullets.effectTimeRemaining > 0.f; }
    bool isFireRateActive() const { return fireRate.effectTimeRemaining > 0.f; }

    // 0 = just used, 1 = fully recharged and ready. For UI bars.
    float burstReadiness() const { return readiness(burst); }
    float bigBulletsReadiness() const { return readiness(bigBullets); }
    float fireRateReadiness() const { return readiness(fireRate); }

private:
    struct PowerupState {
        float cooldownDuration;
        float effectDuration = 0.f; // 0 for instant powerups (Burst)
        float cooldownRemaining = 0.f;
        float effectTimeRemaining = 0.f;
    };

    static void tickCooldown(PowerupState& s, float dt) {
        if (s.cooldownRemaining > 0.f) s.cooldownRemaining -= dt;
        if (s.effectTimeRemaining > 0.f) s.effectTimeRemaining -= dt;
    }

    static float readiness(const PowerupState& s) {
        if (s.cooldownDuration <= 0.f) return 1.f;
        float r = 1.f - (s.cooldownRemaining / s.cooldownDuration);
        return std::max(0.f, std::min(1.f, r));
    }

    // Edge-detects a key press so holding the key down doesn't
    // re-trigger the powerup every single frame.
    static bool justPressed(sf::Keyboard::Key key, bool& wasDownFlag) {
        bool down = sf::Keyboard::isKeyPressed(key);
        bool result = down && !wasDownFlag;
        wasDownFlag = down;
        return result;
    }

    PowerupState burst{/*cooldownDuration=*/30.f};
    PowerupState bigBullets{/*cooldownDuration=*/30.f, /*effectDuration=*/8.f};
    PowerupState fireRate{/*cooldownDuration=*/30.f, /*effectDuration=*/8.f};

    bool burstKeyWasDown = false;
    bool bigBulletsKeyWasDown = false;
    bool fireRateKeyWasDown = false;
};
