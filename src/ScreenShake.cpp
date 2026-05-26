#include "ScreenShake.hpp"
#include <cmath>

ScreenShake::ScreenShake() : duration(0), intensity(0), currentTime(0), isShaking(false),
    rng(std::random_device{}()), dist(-1.0f, 1.0f) {
}

ScreenShake::~ScreenShake() {
}

void ScreenShake::start(float shakeDuration, float shakeIntensity) {
    duration = shakeDuration;
    intensity = shakeIntensity;
    currentTime = 0;
    isShaking = true;
}

void ScreenShake::update(float deltaTime) {
    if (!isShaking) return;
    
    currentTime += deltaTime;
    if (currentTime >= duration) {
        isShaking = false;
    }
}

sf::Vector2f ScreenShake::getOffset() {
    if (!isShaking) return sf::Vector2f(0, 0);
    
    float progress = currentTime / duration;
    float currentIntensity = intensity * (1.0f - progress);
    
    float x = dist(rng) * currentIntensity;
    float y = dist(rng) * currentIntensity;
    
    return sf::Vector2f(x, y);
}
