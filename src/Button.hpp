#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <functional>
#include <optional>

class Button {
private:
    sf::RectangleShape background;
    sf::Text text;

    bool selected;
    bool hovered;
    bool isPressed;
    bool isDisabled;

    sf::Color normalFill;
    sf::Color hoverFill;
    sf::Color selectedFill;
    sf::Color disabledFill;

    sf::Color normalOutline;
    sf::Color hoverOutline;
    sf::Color selectedOutline;
    sf::Color disabledOutline;

    // Lerped current colors (used by animated update)
    sf::Color currentFill;
    sf::Color currentOutline;

    // Scale animation
    float currentScale;
    float targetScale;
    float animationSpeed;   // lerp speed in units/sec (default 8)

    // Timers
    float glowTimer;        // drives outline pulse on selected state
    float pressTimer;       // auto-releases pressed state after 0.15s

    // Stable hit-test bounds (unaffected by scale animation)
    sf::Vector2f originalPosition;
    sf::Vector2f originalSize;

    // Optional click callback
    std::function<void()> onClickCallback;

public:
    Button(
        const sf::Font& font,
        const std::string& label,
        unsigned int characterSize,
        sf::Vector2f size,
        sf::Vector2f position
    );

    void setText(const std::string& str);

    void setSelected(bool value);
    void setHovered(bool value);
    void setDisabled(bool value);
    void setCallback(std::function<void()> cb);

    bool getDisabled() const;

    bool contains(sf::Vector2f mousePos) const;

    // Trigger press animation; fires callback if set
    void press();

    // Legacy instant-snap update (backward-compatible, no deltaTime)
    void update();

    // Animated update — call once per frame from state update()
    void update(float deltaTime);

    void render(sf::RenderWindow& window);

    sf::Vector2f getPosition() const;
    sf::FloatRect getBounds() const;
};
