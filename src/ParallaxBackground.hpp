#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>

class ParallaxBackground {
private:
    struct Layer {
        sf::Texture texture;
        sf::Sprite sprite1;
        sf::Sprite sprite2;
        float speed;
        float offset;
        
        // Constructor to properly initialize sprites
        Layer() : speed(0), offset(0) {
            // Sprites will be set after texture loads
        }
    };
    
    std::vector<Layer> layers;
    sf::RenderWindow* window;
    
public:
    ParallaxBackground();
    ~ParallaxBackground();
    
    void loadLayer(const std::string& texturePath, float speed);
    void update(float deltaTime);
    void render(sf::RenderWindow& target);
    void setWindow(sf::RenderWindow* win) { window = win; }
};
