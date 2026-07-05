#include <SFML/Graphics.hpp>

int main() {
    // 1. Create the application window (width, height, window title)
    sf::RenderWindow window(sf::VideoMode(800, 600), "Prahaar: Vampire Survivors Clone");

    // 2. The Core Game Loop - runs infinitely 60+ times per second until the window closes
    while (window.isOpen()) {
        
        // 3. EVENT HANDLING: Listen for interactions (like clicking the Close button)
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close(); // Breaks the loop and shuts down the game
            }
        }

        // 4. GAME LOGIC UPDATES: (This is where player movement and enemy tracking will go)

        // 5. RENDERING PIPELINE: Clear old frame -> Draw new frames -> Show to screen
        window.clear(sf::Color(30, 30, 30)); // Clears screen with a sleek dark grey color

        // (Future character drawing commands like window.draw(player) will go here)

        window.display(); // Flips the graphic buffers to update what the player actually sees
    }

    return 0;
}