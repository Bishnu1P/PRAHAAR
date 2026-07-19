#pragma once
#include <cmath>
#include "Entity.hpp"

// Player character. Currently a green circle (placeholder art),
// same visual your friend already set up, now wired into the
// Entity/update/draw structure and given actual keyboard movement.
class Player : public Entity {
public:
    Player() {
        shape.setRadius(20.f);
        shape.setFillColor(sf::Color::Green);
        shape.setOrigin(20.f, 20.f); // origin = center, so position = center point
        setPosition({400.f, 300.f});
        speed = 200.f; // pixels per second (frame-rate independent, unlike a flat per-frame value)
        active = true; // player always exists
    }

    void update(float dt) override {
        velocity = {0.f, 0.f};

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            velocity.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            velocity.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            velocity.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            velocity.x += 1.f;

        // Normalize diagonal movement so it isn't faster than straight movement
        float length = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        if (length != 0.f) {
            velocity /= length;
        }

        position += velocity * speed * dt;
        shape.setPosition(position);
    }

    void draw(sf::RenderWindow& window) override {
        window.draw(shape);
    }

private:
    sf::CircleShape shape;
    float speed;
};
