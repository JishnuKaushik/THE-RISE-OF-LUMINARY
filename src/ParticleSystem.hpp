#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

struct Particle {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float life;
};

struct FloatingText {
    std::string str;
    sf::Color color;
    sf::Vector2f position;
    sf::Vector2f velocity;
    float life;
    float totalLife;
};

class ParticleSystem {
private:
    std::vector<Particle> particles;
    std::vector<FloatingText> floatingTexts;
    const sf::Font* font;

public:
    ParticleSystem();
    ~ParticleSystem();

    void setFont(const sf::Font& f) { font = &f; }

    void spawnExplosion(sf::Vector2f position, sf::Color color);
    void spawnSparkles(sf::Vector2f position);
    void spawnFire(sf::Vector2f position);
    void spawnSmoke(sf::Vector2f position);
    void spawnFloatingText(sf::Vector2f pos, const std::string& text, sf::Color color);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    void clear();
};
