#pragma once
#include <SFML/Graphics.hpp>
#include "Player.hpp"
#include "Enemy.hpp"
#include "EntityPool.hpp"
#include "EnemySpawner.hpp"

class Game {
public:
    Game()
        : window(sf::VideoMode(1280, 720), "PRAHAAR"),
          fixedTimeStep(1.f / 60.f),
          enemyPool(200), // pool size: max simultaneous enemies. Raise if you see enemies stop spawning.
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
        // Later (Step 4): collision detection between player/enemies/projectiles here.
    }

    void render() {
        window.clear(sf::Color(30, 30, 40));
        player.draw(window);
        enemyPool.drawAll(window);
        window.display();
    }

    sf::RenderWindow window;
    const float fixedTimeStep;
    float elapsedTime = 0.f;

    Player player;
    EntityPool<Enemy> enemyPool;
    EnemySpawner spawner;
};
