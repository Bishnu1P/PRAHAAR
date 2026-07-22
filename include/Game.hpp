#pragma once
#include <cmath>
#include <random>
#include <SFML/Graphics.hpp>
#include "Player.hpp"
#include "Enemy.hpp"
#include "Boss.hpp"
#include "Projectile.hpp"
#include "XPOrb.hpp"
#include "Particle.hpp"
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
          particlePool(400),
          spawner(enemyPool, window.getSize()),
          rng(std::random_device{}())
    {
        window.setFramerateLimit(144);
        defaultView = window.getDefaultView();
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

        // Normal enemies stop spawning during a boss fight — this is
        // the "one-on-one" part of the idea. Enemies already on screen
        // were cleared the moment the boss appeared (see startBossFight).
        if (!bossActive) {
            spawner.update(dt, elapsedTime, progression.getLevel(), player);
        }

        enemyPool.updateAll(dt);
        projectilePool.updateAll(dt);
        xpOrbPool.updateAll(dt);
        particlePool.updateAll(dt);
        if (shakeTimer > 0.f) shakeTimer -= dt;
        if (bossActive) boss.update(dt);

        weapon.update(dt, player, enemyPool, projectilePool, powerups, &boss, bossActive);

        bool burstTriggered = powerups.update(dt);
        if (burstTriggered) applyBurst();

        handleCollisions();

        if (!bossActive && normalKillCount >= killsPerBoss) {
            startBossFight();
        }
    }

    static bool circlesOverlap(sf::Vector2f posA, float radiusA, sf::Vector2f posB, float radiusB) {
        sf::Vector2f diff = posA - posB;
        float distSq = diff.x * diff.x + diff.y * diff.y;
        float radiusSum = radiusA + radiusB;
        return distSq <= radiusSum * radiusSum;
    }

    void startBossFight() {
        // Clear the board — this is the "after finishing normal enemies"
        // part. Any enemies still alive when the threshold is hit
        // despawn so the fight is genuinely one-on-one.
        for (Enemy* enemy : enemyPool.getActive()) {
            enemy->setActive(false);
        }

        sf::Vector2f spawnPos(static_cast<float>(window.getSize().x) / 2.f, -60.f);
        boss.reset(spawnPos, &player, difficultyStage);
        bossActive = true;
        normalKillCount = 0;
    }

    void applyBurst() {
        for (Enemy* enemy : enemyPool.getActive()) {
            if (!enemy->isActive()) continue;
            sf::Vector2f diff = enemy->getPosition() - player.getPosition();
            float distSq = diff.x * diff.x + diff.y * diff.y;
            if (distSq <= burstRadius * burstRadius) {
                sf::Vector2f deathPos = enemy->getPosition();
                enemy->takeDamage(burstDamage);
                if (!enemy->isActive()) {
                    normalKillCount++;
                    spawnXPOrb(deathPos);
                    spawnDeathParticles(deathPos, sf::Color(255, 160, 60), 10);
                }
            }
        }

        // Burst also hits the boss if one's active and in range.
        if (bossActive) {
            sf::Vector2f diff = boss.getPosition() - player.getPosition();
            float distSq = diff.x * diff.x + diff.y * diff.y;
            if (distSq <= burstRadius * burstRadius) {
                boss.takeDamage(burstDamage);
                if (!boss.isActive()) onBossDefeated();
            }
        }

        burstFlashTimer = 0.15f;
        triggerScreenShake(10.f, 0.2f);
    }

    void handleCollisions() {
        auto activeEnemies = enemyPool.getActive();
        auto activeProjectiles = projectilePool.getActive();
        auto activeOrbs = xpOrbPool.getActive();

        // Projectile vs Enemy
        for (Projectile* proj : activeProjectiles) {
            if (!proj->isActive()) continue;

            for (Enemy* enemy : activeEnemies) {
                if (!enemy->isActive()) continue;

                if (circlesOverlap(proj->getPosition(), proj->getRadius(), enemy->getPosition(), enemy->getRadius())) {
                    sf::Vector2f deathPos = enemy->getPosition();
                    enemy->takeDamage(proj->getDamage());
                    proj->setActive(false);

                    if (!enemy->isActive()) {
                        normalKillCount++;
                        spawnXPOrb(deathPos);
                        spawnDeathParticles(deathPos, sf::Color(220, 80, 80), 8);
                    }
                    break;
                }
            }

            // Projectile vs Boss
            if (proj->isActive() && bossActive) {
                if (circlesOverlap(proj->getPosition(), proj->getRadius(), boss.getPosition(), boss.getRadius())) {
                    boss.takeDamage(proj->getDamage());
                    proj->setActive(false);
                    if (!boss.isActive()) onBossDefeated();
                }
            }
        }

        // Enemy vs Player
        for (Enemy* enemy : activeEnemies) {
            if (!enemy->isActive()) continue;
            if (circlesOverlap(enemy->getPosition(), enemy->getRadius(), player.getPosition(), player.getRadius())) {
                if (player.takeDamage(enemy->getTouchDamage())) {
                    triggerScreenShake(6.f, 0.15f);
                }
            }
        }

        // Boss vs Player
        if (bossActive && circlesOverlap(boss.getPosition(), boss.getRadius(), player.getPosition(), player.getRadius())) {
            if (player.takeDamage(boss.getTouchDamage())) {
                triggerScreenShake(10.f, 0.2f);
            }
        }

        // XPOrb vs Player
        for (XPOrb* orb : activeOrbs) {
            if (!orb->isActive()) continue;
            if (circlesOverlap(orb->getPosition(), orb->getRadius(), player.getPosition(), player.getRadius())) {
                float value = orb->getValue();
                orb->setActive(false);
                progression.addXP(value, [this]() { applyRandomUpgrade(); });
            }
        }
    }

    // Boss defeated: fight ends, next boss will be tougher, and per
    // the original idea, the player's level rises on the kill. We
    // grant exactly enough XP to force one level-up (also awarding
    // the usual random upgrade), rather than inventing a separate
    // "boss level" system disconnected from normal progression.
    void onBossDefeated() {
        bossActive = false;
        difficultyStage++;

        sf::Vector2f pos = boss.getPosition();
        for (int i = 0; i < 5; ++i) {
            sf::Vector2f offset(static_cast<float>((i - 2) * 15), static_cast<float>((i % 2 == 0) ? 15 : -15));
            spawnXPOrb(pos + offset);
        }

        spawnDeathParticles(pos, sf::Color(220, 60, 220), 30);
        triggerScreenShake(16.f, 0.35f);

        progression.addXP(progression.xpToNextLevel(), [this]() { applyRandomUpgrade(); });
    }

    void spawnXPOrb(sf::Vector2f pos) {
        XPOrb* orb = xpOrbPool.spawn();
        if (orb) {
            orb->reset(pos, xpPerEnemy, &player);
        }
    }

    // Small burst of fading particles flying outward — used for enemy
    // and boss deaths so a kill has some visual "pop" beyond the
    // entity just vanishing.
    void spawnDeathParticles(sf::Vector2f pos, sf::Color color, int count) {
        std::uniform_real_distribution<float> angleDist(0.f, 6.2831853f); // 0 to 2*pi
        std::uniform_real_distribution<float> speedDist(60.f, 160.f);

        for (int i = 0; i < count; ++i) {
            Particle* p = particlePool.spawn();
            if (!p) break; // pool exhausted, skip remaining silently

            float angle = angleDist(rng);
            float speed = speedDist(rng);
            sf::Vector2f vel(std::cos(angle) * speed, std::sin(angle) * speed);
            p->reset(pos, vel, color, 0.4f);
        }
    }

    void triggerScreenShake(float magnitude, float duration) {
        // Overwrite rather than accumulate — a second hit while already
        // shaking just refreshes the effect instead of stacking forever.
        shakeMagnitude = magnitude;
        shakeTimer = duration;
    }

    void applyRandomUpgrade() {
        std::uniform_int_distribution<int> dist(0, 3);
        switch (dist(rng)) {
            case 0: player.increaseMaxHealth(20.f); break;
            case 1: player.increaseSpeed(20.f); break;
            case 2: weapon.increaseDamage(4.f); break;
            default: weapon.increaseFireRate(0.3f); break;
        }
    }

    void render() {
        window.clear(sf::Color(30, 30, 40));

        // Apply screen shake to world-space rendering only. UI (bars)
        // gets reset back to the default view further down, so shake
        // never makes health/XP bars jitter or drift off-screen.
        sf::View worldView = defaultView;
        if (shakeTimer > 0.f) {
            std::uniform_real_distribution<float> offsetDist(-shakeMagnitude, shakeMagnitude);
            worldView.move(offsetDist(rng), offsetDist(rng));
        }
        window.setView(worldView);

        player.draw(window);
        enemyPool.drawAll(window);
        if (bossActive) boss.draw(window);
        projectilePool.drawAll(window);
        xpOrbPool.drawAll(window);
        particlePool.drawAll(window);
        drawBurstFlash();

        window.setView(defaultView);
        drawXPBar();
        drawPowerupBars();
        if (bossActive) drawBossHealthBar();

        window.display();
    }

    void drawBurstFlash() {
        if (burstFlashTimer <= 0.f) return;
        burstFlashTimer -= 1.f / 144.f;

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

    void drawPowerupBars() {
        float barWidth = 90.f;
        float barHeight = 8.f;
        float startX = 20.f;
        float startY = static_cast<float>(window.getSize().y) - 60.f;

        drawOneBar(startX, startY, barWidth, barHeight, powerups.burstReadiness(), sf::Color(255, 140, 60));
        drawOneBar(startX, startY + 16.f, barWidth, barHeight, powerups.bigBulletsReadiness(), sf::Color(255, 200, 80));
        drawOneBar(startX, startY + 32.f, barWidth, barHeight, powerups.fireRateReadiness(), sf::Color(120, 220, 255));
    }

    void drawOneBar(float x, float y, float width, float height, float ratio, sf::Color color) {
        sf::RectangleShape background(sf::Vector2f(width, height));
        background.setPosition(x, y);
        background.setFillColor(sf::Color(60, 60, 60));

        sf::RectangleShape fill(sf::Vector2f(width * ratio, height));
        fill.setPosition(x, y);
        fill.setFillColor(color);

        window.draw(background);
        window.draw(fill);
    }

    // Red bar top-center of screen while a boss is active — the classic
    // "boss health bar" convention, distinct from the player's own bar.
    void drawBossHealthBar() {
        float barWidth = 500.f;
        float barHeight = 14.f;
        float x = (static_cast<float>(window.getSize().x) - barWidth) / 2.f;
        float y = 50.f;
        float ratio = boss.getHealth() / boss.getMaxHealth();

        sf::RectangleShape background(sf::Vector2f(barWidth, barHeight));
        background.setPosition(x, y);
        background.setFillColor(sf::Color(60, 20, 60));

        sf::RectangleShape fill(sf::Vector2f(barWidth * ratio, barHeight));
        fill.setPosition(x, y);
        fill.setFillColor(sf::Color(220, 60, 220));

        window.draw(background);
        window.draw(fill);
    }

    sf::RenderWindow window;
    const float fixedTimeStep;
    float elapsedTime = 0.f;
    float xpPerEnemy = 3.f;

    Player player;
    EntityPool<Enemy> enemyPool;
    EntityPool<Projectile> projectilePool;
    EntityPool<XPOrb> xpOrbPool;
    EntityPool<Particle> particlePool;
    EnemySpawner spawner;
    Weapon weapon;
    Progression progression;
    PowerupSystem powerups;
    std::mt19937 rng;

    float burstRadius = 180.f;
    float burstDamage = 50.f;
    float burstFlashTimer = 0.f;

    // --- Screen shake ---
    sf::View defaultView;
    float shakeTimer = 0.f;
    float shakeMagnitude = 0.f;

    // --- Boss fight tracking ---
    Boss boss;
    bool bossActive = false;
    int normalKillCount = 0;
    int killsPerBoss = 15;      // tune this: how many normal kills before a boss appears
    int difficultyStage = 1;    // increments each boss defeat, making the next one tougher
};
