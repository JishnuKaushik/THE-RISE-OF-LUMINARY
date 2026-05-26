#include "ParallaxBackground.hpp"
#include <iostream>

ParallaxBackground::ParallaxBackground() : window(nullptr) {
}

ParallaxBackground::~ParallaxBackground() {
}

void ParallaxBackground::loadLayer(const std::string& texturePath, float speed) {
    Layer layer;
    layer.speed = speed;
    layer.offset = 0;
    
    if (!layer.texture.loadFromFile(texturePath)) {
        std::cerr << "Failed to load parallax layer: " << texturePath << std::endl;
        // Create a fallback colored texture
        sf::Image fallback;
        fallback.create({1920, 1080}, sf::Color(20, 15, 40));
        layer.texture.loadFromImage(fallback);
    }
    
    layer.sprite1.setTexture(layer.texture);
    layer.sprite2.setTexture(layer.texture);
    layer.sprite1.setScale(sf::Vector2f(1.0f, 1.0f));
    layer.sprite2.setScale(sf::Vector2f(1.0f, 1.0f));
    layer.sprite2.setPosition(sf::Vector2f(static_cast<float>(layer.texture.getSize().x), 0));
    
    layers.push_back(std::move(layer));
    std::cout << "Loaded parallax layer with speed: " << speed << std::endl;
}

void ParallaxBackground::update(float deltaTime) {
    for (auto& layer : layers) {
        layer.offset += layer.speed * deltaTime * 100;
        
        // Wrap around for seamless scrolling
        if (layer.offset >= layer.texture.getSize().x) {
            layer.offset -= layer.texture.getSize().x;
        }
        
        layer.sprite1.setPosition(sf::Vector2f(-layer.offset, 0));
        layer.sprite2.setPosition(sf::Vector2f(static_cast<float>(layer.texture.getSize().x) - layer.offset, 0));
    }
}

void ParallaxBackground::render(sf::RenderWindow& target) {
    for (const auto& layer : layers) {
        target.draw(layer.sprite1);
        target.draw(layer.sprite2);
    }
}
