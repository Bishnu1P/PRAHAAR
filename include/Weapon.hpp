#pragma once
#include <cmath>
#include <limits>
#include "Player.hpp"
#include "Enemy.hpp"
#include "Projectile.hpp"
#include "EntityPool.hpp"

// The "Dagger" starter weapon from docs/design-doc.md: fires
// automatically at the nearest enemy, no manual aiming needed.
class Weapon {
public:
    void update(float dt, const Player& player, EntityPool<Enemy>& enemies, EntityPool<Projectile>& projectiles) {
        cooldownTimer += dt;

        if (cooldownTimer >= 1.f / fireRate) {
            Enemy* target = findNearest(player, enemies);
            if (target) {
                fireAt(player, *target, projectiles);
                cooldownTimer = 0.f;
            }
            // If no target exists, we don't reset the timer — so the very
            // next frame an enemy appears, it fires immediately rather
            // than waiting out a full extra cooldown.
        }
    }

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

    void fireAt(const Player& player, const Enemy& target, EntityPool<Projectile>& projectiles) {
        Projectile* proj = projectiles.spawn();
        if (!proj) return; // pool exhausted, skip silently

        sf::Vector2f dir = target.getPosition() - player.getPosition();
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (length > 0.001f) dir /= length;

        proj->reset(player.getPosition(), dir, projectileSpeed, damage);
    }

    float fireRate = 1.5f;         // shots per second, matches Dagger in design doc
    float damage = 8.f;            // matches Dagger base damage
    float projectileSpeed = 500.f;
    float cooldownTimer = 0.f;
};
