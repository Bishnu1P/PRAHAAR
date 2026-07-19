#pragma once
#include <cmath>
#include <limits>
#include "Player.hpp"
#include "Enemy.hpp"
#include "Projectile.hpp"
#include "EntityPool.hpp"
#include "PowerupSystem.hpp"

// The "Dagger" starter weapon from docs/design-doc.md: fires
// automatically at the nearest enemy, no manual aiming needed.
// Also responds to the K (big bullets) and L (rapid fire) powerups.
class Weapon {
public:
    void update(float dt, const Player& player, EntityPool<Enemy>& enemies,
                EntityPool<Projectile>& projectiles, const PowerupSystem& powerups) {
        cooldownTimer += dt;

        float effectiveFireRate = powerups.isFireRateActive() ? fireRate * 2.f : fireRate;

        if (cooldownTimer >= 1.f / effectiveFireRate) {
            Enemy* target = findNearest(player, enemies);
            if (target) {
                fireAt(player, *target, projectiles, powerups);
                cooldownTimer = 0.f;
            }
        }
    }

    // --- Upgrades (applied on level-up, see Game::applyRandomUpgrade) ---
    void increaseDamage(float amount) { damage += amount; }
    void increaseFireRate(float amount) { fireRate += amount; }

private:
    Enemy* findNearest(const Player& player, EntityPool<Enemy>& enemies) {
        Enemy* nearest = nullptr;
        float nearestDistSq = std::numeric_limits<float>::max();

        for (Enemy* enemy : enemies.getActive()) {
            sf::Vector2f diff = enemy->getPosition() - player.getPosition();
            float distSq = diff.x * diff.x + diff.y * diff.y;
            if (distSq < nearestDistSq) {
                nearestDistSq = distSq;
                nearest = enemy;
            }
        }
        return nearest;
    }

    void fireAt(const Player& player, const Enemy& target, EntityPool<Projectile>& projectiles, const PowerupSystem& powerups) {
        Projectile* proj = projectiles.spawn();
        if (!proj) return; // pool exhausted, skip silently

        sf::Vector2f dir = target.getPosition() - player.getPosition();
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (length > 0.001f) dir /= length;

        bool bigBullets = powerups.isBigBulletsActive();
        float effectiveDamage = bigBullets ? damage * 2.5f : damage;
        float radiusMultiplier = bigBullets ? 2.5f : 1.f;

        proj->reset(player.getPosition(), dir, projectileSpeed, effectiveDamage, radiusMultiplier);
    }

    float fireRate = 1.5f;         // shots per second, matches Dagger in design doc
    float damage = 8.f;            // matches Dagger base damage
    float projectileSpeed = 500.f;
    float cooldownTimer = 0.f;
};
