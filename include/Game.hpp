#pragma once
#include <SFML/Graphics.hpp>
#include "Player.hpp"

class Game {
public:
    Game()
        : window(sf::VideoMode(1280, 720), "PRAHAAR"),
          fixedTimeStep(1.f / 60.f) // update logic 60 times per second, always
    {
        window.setFramerateLimit(144); // render can go faster than logic updates
    }

    void run() {
        sf::Clock clock;
        float accumulator = 0.f;

        while (window.isOpen()) {
            handleEvents();

            // How much real time passed since last frame
            float frameTime = clock.restart().asSeconds();

            // Safety cap: if the game freezes/lags badly (e.g. window dragged),
            // don't try to "catch up" hundreds of updates at once.
            if (frameTime > 0.25f) frameTime = 0.25f;

            accumulator += frameTime;

            // Run fixed-size update steps until we've caught up to real time.
            // This decouples game logic speed from render/frame rate.
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
        player.update(dt);
        // Later: enemyPool.updateAll(dt); projectilePool.updateAll(dt); collisions, etc.
    }

    void render() {
        window.clear(sf::Color(30, 30, 40));
        player.draw(window);
        // Later: enemyPool.drawAll(window); projectilePool.drawAll(window);
        window.display();
    }

    sf::RenderWindow window;
    const float fixedTimeStep;
    Player player;
};
