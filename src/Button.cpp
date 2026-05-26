#include "Button.hpp"

Button::Button(
    const sf::Font& font,
    const std::string& label,
    unsigned int characterSize,
    sf::Vector2f size,
    sf::Vector2f position
)
    : text(font, label, characterSize),
    selected(false),
    hovered(false)
{
    background.setSize(size);
    background.setPosition(position);

    normalFill = sf::Color(10, 10, 20, 170);
    hoverFill = sf::Color(30, 30, 50, 210);
    selectedFill = sf::Color(40, 40, 70, 230);

    normalOutline = sf::Color(255, 220, 120, 100);
    hoverOutline = sf::Color(255, 220, 120, 180);
    selectedOutline = sf::Color(255, 235, 160);

    background.setFillColor(normalFill);
    background.setOutlineColor(normalOutline);
    background.setOutlineThickness(1);

        text.setPosition(sf::Vector2f(
        position.x + 20,
        position.y + 8
    ));
    text.setFillColor(sf::Color::White);
}

void Button::setText(const std::string& str) {
    text.setString(str);
}

void Button::setSelected(bool value) {
    selected = value;
}

void Button::setHovered(bool value) {
    hovered = value;
}

bool Button::contains(sf::Vector2f mousePos) const {
    return background.getGlobalBounds().contains(mousePos);
}

void Button::update() {
    if (selected) {
        background.setFillColor(selectedFill);
        background.setOutlineColor(selectedOutline);
        background.setOutlineThickness(2);
    }
    else if (hovered) {
        background.setFillColor(hoverFill);
        background.setOutlineColor(hoverOutline);
        background.setOutlineThickness(2);
    }
    else {
        background.setFillColor(normalFill);
        background.setOutlineColor(normalOutline);
        background.setOutlineThickness(1);
    }
}

void Button::render(sf::RenderWindow& window) {
    window.draw(background);
    window.draw(text);
}

sf::Vector2f Button::getPosition() const {
    return background.getPosition();
}
sf::FloatRect Button::getBounds() const {
    return background.getGlobalBounds();
}
