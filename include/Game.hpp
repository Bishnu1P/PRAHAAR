#pragma once
#include <cmath>
#include <SFML/Graphics.hpp>
#include "Player.hpp"
#include "Enemy.hpp"
#include "Projectile.hpp"
#include "EntityPool.hpp"
#include "EnemySpawner.hpp"
#include "Weapon.hpp"

class Game {
public:
    Game()
        : window(sf::VideoMode(1280, 720), "PRAHAAR"),
          fixedTimeStep(1.f / 60.f),
          enemyPool(200),
          projectilePool(300), // plenty of headroom; a fast fire rate can have many in flight
          spawner(enemyPool, window.getSize())
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
        spawner.update(dt, elapsedTime, player);
        enemyPool.updateAll(dt);
        projectilePool.updateAll(dt);
        weapon.update(dt, player, enemyPool, projectilePool);

        handleCollisions();
    }

    // Circle-vs-circle collision: two things overlap if the distance
    // between their centers is less than the sum of their radii.
    static bool circlesOverlap(sf::Vector2f posA, float radiusA, sf::Vector2f posB, float radiusB) {
        sf::Vector2f diff = posA - posB;
        float distSq = diff.x * diff.x + diff.y * diff.y;
        float radiusSum = radiusA + radiusB;
        return distSq <= radiusSum * radiusSum;
    }

    void handleCollisions() {
        auto activeEnemies = enemyPool.getActive();
        auto activeProjectiles = projectilePool.getActive();

        // Projectile vs Enemy: projectile deals damage and disappears on hit.
        for (Projectile* proj : activeProjectiles) {
            if (!proj->isActive()) continue; // may have been deactivated earlier this same loop

            for (Enemy* enemy : activeEnemies) {
                if (!enemy->isActive()) continue; // may have died to an earlier projectile this frame

                if (circlesOverlap(proj->getPosition(), proj->getRadius(), enemy->getPosition(), enemy->getRadius())) {
                    enemy->takeDamage(proj->getDamage());
                    proj->setActive(false);
                    break; // this projectile is spent, stop checking other enemies
                }
            }
        }

        // Enemy vs Player: enemy deals touch damage (player has a brief
        // invulnerability window after each hit, handled inside Player).
        for (Enemy* enemy : activeEnemies) {
            if (!enemy->isActive()) continue;

            if (circlesOverlap(enemy->getPosition(), enemy->getRadius(), player.getPosition(), player.getRadius())) {
                player.takeDamage(enemy->getTouchDamage());
            }
        }
    }

    void render() {
        window.clear(sf::Color(30, 30, 40));
        player.draw(window);
        enemyPool.drawAll(window);
        projectilePool.drawAll(window);
        window.display();
    }

    sf::RenderWindow window;
    const float fixedTimeStep;
    float elapsedTime = 0.f;

    Player player;
    EntityPool<Enemy> enemyPool;
    EntityPool<Projectile> projectilePool;
    EnemySpawner spawner;
    Weapon weapon;
};
