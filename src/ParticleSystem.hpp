#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

struct Particle {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float life;
};

class ParticleSystem {
private:
    std::vector<Particle> particles;
    
public:
    ParticleSystem();
    ~ParticleSystem();
    
    void spawnExplosion(sf::Vector2f position, sf::Color color);
    void spawnSparkles(sf::Vector2f position);
    void spawnFire(sf::Vector2f position);
    void spawnSmoke(sf::Vector2f position);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);  // Fixed signature
    void clear();
};
