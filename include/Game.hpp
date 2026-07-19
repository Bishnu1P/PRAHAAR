#pragma once
#include <cmath>
#include <random>
#include <SFML/Graphics.hpp>
#include "Player.hpp"
#include "Enemy.hpp"
#include "Projectile.hpp"
#include "XPOrb.hpp"
#include "EntityPool.hpp"
#include "EnemySpawner.hpp"
#include "Weapon.hpp"
#include "Progression.hpp"
#include "PowerupSystem.hpp"

class Game {
public:
    Game()
        : window(sf::VideoMode(1280, 720), "PRAHAAR"),
          fixedTimeStep(1.f / 60.f),
          enemyPool(200),
          projectilePool(300),
          xpOrbPool(200),
          spawner(enemyPool, window.getSize()),
          rng(std::random_device{}())
    {
        window.setFramerateLimit(144);
    }

    void run() {
        sf::Clock clock;
        float accumulator = 0.f;

        while (window.isOpen()) {
            handleEvents();

            float frameTime = clock.restart().asSeconds();
            if (frameTime > 0.25f) frameTime = 0.25f;

            accumulator += frameTime;

            while (accumulator >= fixedTimeStep) {
                update(fixedTimeStep);
                accumulator -= fixedTimeStep;
            }

            render();
        }
    }

private:
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
    }

    void update(float dt) {
        elapsedTime += dt;

        player.update(dt);
        spawner.update(dt, elapsedTime, progression.getLevel(), player);
        enemyPool.updateAll(dt);
        projectilePool.updateAll(dt);
        xpOrbPool.updateAll(dt);
        weapon.update(dt, player, enemyPool, projectilePool, powerups);

        bool burstTriggered = powerups.update(dt);
        if (burstTriggered) {
            applyBurst();
        }

        handleCollisions();
    }

    static bool circlesOverlap(sf::Vector2f posA, float radiusA, sf::Vector2f posB, float radiusB) {
        sf::Vector2f diff = posA - posB;
        float distSq = diff.x * diff.x + diff.y * diff.y;
        float radiusSum = radiusA + radiusB;
        return distSq <= radiusSum * radiusSum;
    }

    // "J" powerup: instant AoE damage to everything near the player.
    void applyBurst() {
        for (Enemy* enemy : enemyPool.getActive()) {
            if (!enemy->isActive()) continue;

            sf::Vector2f diff = enemy->getPosition() - player.getPosition();
            float distSq = diff.x * diff.x + diff.y * diff.y;

            if (distSq <= burstRadius * burstRadius) {
                sf::Vector2f deathPos = enemy->getPosition();
                enemy->takeDamage(burstDamage);
                if (!enemy->isActive()) {
                    spawnXPOrb(deathPos);
                }
            }
        }
        burstFlashTimer = 0.15f; // brief visual flash, see drawBurstFlash()
    }

    void handleCollisions() {
        auto activeEnemies = enemyPool.getActive();
        auto activeProjectiles = projectilePool.getActive();
        auto activeOrbs = xpOrbPool.getActive();

        for (Projectile* proj : activeProjectiles) {
            if (!proj->isActive()) continue;

            for (Enemy* enemy : activeEnemies) {
                if (!enemy->isActive()) continue;

                if (circlesOverlap(proj->getPosition(), proj->getRadius(), enemy->getPosition(), enemy->getRadius())) {
                    sf::Vector2f deathPos = enemy->getPosition();
                    enemy->takeDamage(proj->getDamage());
                    proj->setActive(false);

                    if (!enemy->isActive()) {
                        spawnXPOrb(deathPos);
                    }
                    break;
                }
            }
        }

        for (Enemy* enemy : activeEnemies) {
            if (!enemy->isActive()) continue;

            if (circlesOverlap(enemy->getPosition(), enemy->getRadius(), player.getPosition(), player.getRadius())) {
                player.takeDamage(enemy->getTouchDamage());
            }
        }

        for (XPOrb* orb : activeOrbs) {
            if (!orb->isActive()) continue;

            if (circlesOverlap(orb->getPosition(), orb->getRadius(), player.getPosition(), player.getRadius())) {
                float value = orb->getValue();
                orb->setActive(false);
                progression.addXP(value, [this]() { applyRandomUpgrade(); });
            }
        }
    }

    void spawnXPOrb(sf::Vector2f pos) {
        XPOrb* orb = xpOrbPool.spawn();
        if (orb) {
            orb->reset(pos, xpPerEnemy, &player);
        }
    }

    void applyRandomUpgrade() {
        std::uniform_int_distribution<int> dist(0, 3);
        switch (dist(rng)) {
            case 0: player.increaseMaxHealth(20.f); break;
            case 1: player.increaseSpeed(20.f); break;
            case 2: weapon.increaseDamage(4.f); break;
            default: weapon.increaseFireRate(0.3f); break;
        }
        // Note: this picks and applies a bonus instantly for now.
        // Step 7 (UI polish) will turn this into a pause + pick-1-of-3 menu
        // once text rendering is wired up.
    }

    void render() {
        window.clear(sf::Color(30, 30, 40));
        player.draw(window);
        enemyPool.drawAll(window);
        projectilePool.drawAll(window);
        xpOrbPool.drawAll(window);
        drawBurstFlash();
        drawXPBar();
        drawPowerupBars();
        window.display();
    }

    // Brief expanding ring when Burst fires, so the effect is visible
    // even without a proper particle system yet.
    void drawBurstFlash() {
        if (burstFlashTimer <= 0.f) return;
        burstFlashTimer -= 1.f / 144.f; // approximate; fine for a cosmetic flash

        sf::CircleShape ring(burstRadius);
        ring.setOrigin(burstRadius, burstRadius);
        ring.setPosition(player.getPosition());
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineColor(sf::Color(255, 200, 80, 180));
        ring.setOutlineThickness(4.f);
        window.draw(ring);
    }

    void drawXPBar() {
        float margin = 20.f;
        float barWidth = static_cast<float>(window.getSize().x) - margin * 2.f;
        float barHeight = 10.f;
        float ratio = progression.getXP() / progression.xpToNextLevel();

        sf::RectangleShape background(sf::Vector2f(barWidth, barHeight));
        background.setPosition(margin, margin);
        background.setFillColor(sf::Color(60, 60, 60));

        sf::RectangleShape fill(sf::Vector2f(barWidth * ratio, barHeight));
        fill.setPosition(margin, margin);
        fill.setFillColor(sf::Color(80, 220, 230));

        window.draw(background);
        window.draw(fill);

        for (int i = 0; i < progression.getLevel(); ++i) {
            sf::RectangleShape tick(sf::Vector2f(6.f, 6.f));
            tick.setPosition(margin + i * 10.f, margin + barHeight + 6.f);
            tick.setFillColor(sf::Color(200, 200, 200));
            window.draw(tick);
        }
    }

    // 3 small bottom-left bars showing each powerup's recharge state.
    // Full/bright = ready to use, empty/dim = just used, refilling.
    void drawPowerupBars() {
        float barWidth = 90.f;
        float barHeight = 8.f;
        float startX = 20.f;
        float startY = static_cast<float>(window.getSize().y) - 60.f;

        drawOneBar(startX, startY, barWidth, barHeight, powerups.burstReadiness(), sf::Color(255, 140, 60), "J");
        drawOneBar(startX, startY + 16.f, barWidth, barHeight, powerups.bigBulletsReadiness(), sf::Color(255, 200, 80), "K");
        drawOneBar(startX, startY + 32.f, barWidth, barHeight, powerups.fireRateReadiness(), sf::Color(120, 220, 255), "L");
    }

    void drawOneBar(float x, float y, float width, float height, float ratio, sf::Color color, const char*) {
        sf::RectangleShape background(sf::Vector2f(width, height));
        background.setPosition(x, y);
        background.setFillColor(sf::Color(60, 60, 60));

        sf::RectangleShape fill(sf::Vector2f(width * ratio, height));
        fill.setPosition(x, y);
        fill.setFillColor(color);

        window.draw(background);
        window.draw(fill);
        // Note: the J/K/L label isn't drawn yet — no font wired up until
        // Step 7. For now, remember the order: top=J(burst), mid=K(big
        // bullets), bottom=L(rapid fire).
    }

    sf::RenderWindow window;
    const float fixedTimeStep;
    float elapsedTime = 0.f;
    float xpPerEnemy = 3.f;

    Player player;
    EntityPool<Enemy> enemyPool;
    EntityPool<Projectile> projectilePool;
    EntityPool<XPOrb> xpOrbPool;
    EnemySpawner spawner;
    Weapon weapon;
    Progression progression;
    PowerupSystem powerups;
    std::mt19937 rng;

    float burstRadius = 180.f;
    float burstDamage = 50.f;
    float burstFlashTimer = 0.f;
};
