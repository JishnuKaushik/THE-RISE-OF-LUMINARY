#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <optional>

class Button {
private:
    sf::RectangleShape background;
    sf::Text text;

    bool selected;
    bool hovered;

    sf::Color normalFill;
    sf::Color hoverFill;
    sf::Color selectedFill;

    sf::Color normalOutline;
    sf::Color hoverOutline;
    sf::Color selectedOutline;

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

    bool contains(sf::Vector2f mousePos) const;

    void update();

    void render(sf::RenderWindow& window);

    sf::Vector2f getPosition() const;

    sf::FloatRect getBounds() const;

};
