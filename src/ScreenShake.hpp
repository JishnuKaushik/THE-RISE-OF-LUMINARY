#pragma once
#include <SFML/Graphics.hpp>
#include <random>
#include <chrono>

class ScreenShake {
private:
    float duration;
    float intensity;
    float currentTime;
    bool isShaking;
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
    
public:
    ScreenShake();
    ~ScreenShake();
    
    void start(float shakeDuration, float shakeIntensity);
    void update(float deltaTime);
    sf::Vector2f getOffset();
    bool isActive() const { return isShaking; }
};
