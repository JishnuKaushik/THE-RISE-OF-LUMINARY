#include "ParticleSystem.hpp"
#include <cstdlib>
#include <cmath>

ParticleSystem::ParticleSystem() {
}

ParticleSystem::~ParticleSystem() {
    clear();
}

void ParticleSystem::spawnExplosion(sf::Vector2f position, sf::Color color) {
    for (int i = 0; i < 20; i++) {
        Particle p;
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 50 + (rand() % 150);
        p.velocity = sf::Vector2f(cos(angle) * speed, sin(angle) * speed);
        p.shape.setRadius(3 + (rand() % 5));
        p.shape.setFillColor(color);
        p.shape.setPosition(position);
        p.life = 0.5f + (rand() % 100) / 100.0f;
        particles.push_back(p);
    }
}

void ParticleSystem::spawnSparkles(sf::Vector2f position) {
    for (int i = 0; i < 15; i++) {
        Particle p;
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 100 + (rand() % 200);
        p.velocity = sf::Vector2f(cos(angle) * speed, sin(angle) * speed - 100);
        p.shape.setRadius(3);
        p.shape.setFillColor(sf::Color::Yellow);
        p.shape.setPosition(position);
        p.life = 0.6f;
        particles.push_back(p);
    }
}

void ParticleSystem::spawnFire(sf::Vector2f position) {
    for (int i = 0; i < 20; i++) {
        Particle p;
        p.velocity = sf::Vector2f((rand() % 100 - 50), -50 - (rand() % 100));
        p.shape.setRadius(4);
        p.shape.setFillColor(sf::Color(255, 100 + (rand() % 155), 50));
        p.shape.setPosition(sf::Vector2f(position.x + (rand() % 40 - 20), position.y + (rand() % 40 - 20)));
        p.life = 0.5f;
        particles.push_back(p);
    }
}

void ParticleSystem::spawnSmoke(sf::Vector2f position) {
    for (int i = 0; i < 12; i++) {
        Particle p;
        p.velocity = sf::Vector2f((rand() % 100 - 50), -30 - (rand() % 50));
        p.shape.setRadius(5);
        p.shape.setFillColor(sf::Color(80, 70, 60));
        p.shape.setPosition(position);
        p.life = 1.0f;
        particles.push_back(p);
    }
}

void ParticleSystem::update(float deltaTime) {
    for (auto it = particles.begin(); it != particles.end();) {
        it->shape.move(it->velocity * deltaTime);
        it->life -= deltaTime;
        
        if (it->life <= 0) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }
}

void ParticleSystem::render(sf::RenderWindow& window) {
    for (auto& p : particles) {
        window.draw(p.shape);
    }
}

void ParticleSystem::clear() {
    particles.clear();
}
