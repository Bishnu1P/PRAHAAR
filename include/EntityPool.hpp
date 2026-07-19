#pragma once
#include <vector>
#include <memory>
#include "Entity.hpp"

// Manages a fixed-size pool of entities of one type (e.g. all Grunts,
// or all projectiles). Instead of "new Enemy()" / "delete enemy" every
// time one spawns/dies (slow, causes memory fragmentation and stutter),
// we pre-allocate a pool up front and just flip active/inactive flags.
template <typename T>
class EntityPool {
public:
    explicit EntityPool(size_t poolSize) {
        pool.reserve(poolSize);
        for (size_t i = 0; i < poolSize; ++i) {
            pool.push_back(std::make_unique<T>());
            pool.back()->setActive(false);
        }
    }

    // Finds an inactive slot and "spawns" into it, returning a pointer
    // to it (or nullptr if the pool is fully in use — meaning the pool
    // size should be increased).
    T* spawn() {
        for (auto& entity : pool) {
            if (!entity->isActive()) {
                entity->setActive(true);
                return entity.get();
            }
        }
        return nullptr; // pool exhausted
    }

    void updateAll(float dt) {
        for (auto& entity : pool) {
            if (entity->isActive()) {
                entity->update(dt);
            }
        }
    }

    void drawAll(sf::RenderWindow& window) {
        for (auto& entity : pool) {
            if (entity->isActive()) {
                entity->draw(window);
            }
        }
    }

    size_t activeCount() const {
        size_t count = 0;
        for (auto& entity : pool) {
            if (entity->isActive()) ++count;
        }
        return count;
    }

private:
    std::vector<std::unique_ptr<T>> pool;
};
