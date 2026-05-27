#include "Button.hpp"
#include <cmath>
#include <algorithm>

static float lerpF(float a, float b, float t) {
    return a + (b - a) * t;
}

static sf::Color lerpColor(sf::Color a, sf::Color b, float t) {
    return sf::Color(
        static_cast<uint8_t>(lerpF(static_cast<float>(a.r), static_cast<float>(b.r), t)),
        static_cast<uint8_t>(lerpF(static_cast<float>(a.g), static_cast<float>(b.g), t)),
        static_cast<uint8_t>(lerpF(static_cast<float>(a.b), static_cast<float>(b.b), t)),
        static_cast<uint8_t>(lerpF(static_cast<float>(a.a), static_cast<float>(b.a), t))
    );
}

Button::Button(
    const sf::Font& font,
    const std::string& label,
    unsigned int characterSize,
    sf::Vector2f size,
    sf::Vector2f position
)
    : text(font, label, characterSize),
      selected(false),
      hovered(false),
      isPressed(false),
      isDisabled(false),
      currentScale(1.f),
      targetScale(1.f),
      animationSpeed(8.f),
      glowTimer(0.f),
      pressTimer(0.f),
      originalPosition(position),
      originalSize(size)
{
    normalFill    = sf::Color(10, 10, 20, 170);
    hoverFill     = sf::Color(30, 30, 50, 210);
    selectedFill  = sf::Color(40, 40, 70, 230);
    disabledFill  = sf::Color(15, 15, 25, 100);

    normalOutline   = sf::Color(255, 220, 120, 100);
    hoverOutline    = sf::Color(255, 220, 120, 180);
    selectedOutline = sf::Color(255, 235, 160, 255);
    disabledOutline = sf::Color(120, 110, 80, 80);

    currentFill    = normalFill;
    currentOutline = normalOutline;

    // Origin at center so scale animation stays visually centered
    background.setSize(size);
    background.setOrigin(sf::Vector2f(size.x * 0.5f, size.y * 0.5f));
    background.setPosition(sf::Vector2f(position.x + size.x * 0.5f, position.y + size.y * 0.5f));
    background.setFillColor(normalFill);
    background.setOutlineColor(normalOutline);
    background.setOutlineThickness(1.f);

    text.setPosition(sf::Vector2f(position.x + 20.f, position.y + 8.f));
    text.setFillColor(sf::Color::White);
}

// ─── setters ─────────────────────────────────────────────────────────────────

void Button::setText(const std::string& str) {
    text.setString(str);
}

void Button::setSelected(bool value) { selected   = value; }
void Button::setHovered(bool value)  { hovered    = value; }
void Button::setDisabled(bool value) { isDisabled = value; }

void Button::setCallback(std::function<void()> cb) {
    onClickCallback = std::move(cb);
}

bool Button::getDisabled() const { return isDisabled; }

// ─── actions ─────────────────────────────────────────────────────────────────

void Button::press() {
    if (isDisabled) return;
    isPressed  = true;
    pressTimer = 0.15f;
    if (onClickCallback) onClickCallback();
}

// ─── hit testing ─────────────────────────────────────────────────────────────

bool Button::contains(sf::Vector2f mousePos) const {
    return sf::FloatRect(originalPosition, originalSize).contains(mousePos);
}

// ─── legacy no-arg update (instant snap, backward compatible) ────────────────

void Button::update() {
    if (isDisabled) {
        background.setFillColor(disabledFill);
        background.setOutlineColor(disabledOutline);
        background.setOutlineThickness(1.f);
    } else if (selected) {
        background.setFillColor(selectedFill);
        background.setOutlineColor(selectedOutline);
        background.setOutlineThickness(2.f);
    } else if (hovered) {
        background.setFillColor(hoverFill);
        background.setOutlineColor(hoverOutline);
        background.setOutlineThickness(2.f);
    } else {
        background.setFillColor(normalFill);
        background.setOutlineColor(normalOutline);
        background.setOutlineThickness(1.f);
    }
}

// ─── animated update (call once per frame) ───────────────────────────────────

void Button::update(float deltaTime) {
    glowTimer += deltaTime;

    if (isPressed) {
        pressTimer -= deltaTime;
        if (pressTimer <= 0.f) isPressed = false;
    }

    sf::Color targetFill, targetOutline;
    float thickness;

    if (isDisabled) {
        targetFill    = disabledFill;
        targetOutline = disabledOutline;
        targetScale   = 1.0f;
        thickness     = 1.f;
    } else if (isPressed) {
        targetFill    = selectedFill;
        targetOutline = selectedOutline;
        targetScale   = 0.95f;
        thickness     = 3.f;
    } else if (selected) {
        targetFill  = selectedFill;
        targetScale = 1.04f;
        thickness   = 2.f;
        // Outline pulses while selected
        float phase = (std::sin(glowTimer * 3.0f) + 1.f) * 0.5f;
        targetOutline = sf::Color(
            static_cast<uint8_t>(lerpF(180.f, 255.f, phase)),
            static_cast<uint8_t>(lerpF(180.f, 235.f, phase)),
            static_cast<uint8_t>(lerpF(100.f, 160.f, phase)),
            static_cast<uint8_t>(lerpF(160.f, 255.f, phase))
        );
    } else if (hovered) {
        targetFill    = hoverFill;
        targetOutline = hoverOutline;
        targetScale   = 1.05f;
        thickness     = 2.f;
    } else {
        targetFill    = normalFill;
        targetOutline = normalOutline;
        targetScale   = 1.0f;
        thickness     = 1.f;
    }

    // Lerp colors (~125ms at animationSpeed=8)
    float ct = std::min(1.f, deltaTime * animationSpeed);
    currentFill    = lerpColor(currentFill,    targetFill,    ct);
    currentOutline = lerpColor(currentOutline, targetOutline, ct);

    // Lerp scale (~83ms at animationSpeed=8)
    float st = std::min(1.f, deltaTime * animationSpeed * 1.5f);
    currentScale += (targetScale - currentScale) * st;

    background.setFillColor(currentFill);
    background.setOutlineColor(currentOutline);
    background.setOutlineThickness(thickness);
    background.setScale(sf::Vector2f(currentScale, currentScale));

    text.setFillColor(isDisabled ? sf::Color(180, 180, 180, 120) : sf::Color::White);
}

// ─── render ──────────────────────────────────────────────────────────────────

void Button::render(sf::RenderWindow& window) {
    window.draw(background);
    window.draw(text);
}

// ─── accessors ───────────────────────────────────────────────────────────────

sf::Vector2f Button::getPosition() const {
    return originalPosition;
}

sf::FloatRect Button::getBounds() const {
    return sf::FloatRect(originalPosition, originalSize);
}
