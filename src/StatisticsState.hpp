#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Game;

class StatisticsState {
private:
    Game* game;

    sf::Text* titleText;
    sf::Text* overallText;
    sf::Text* subjectText;
    sf::Text* streakText;
    sf::Text* navText;

    sf::RectangleShape* bgPanel;
    sf::RectangleShape* overallPanel;
    sf::RectangleShape* subjectPanel;

    void buildStats();
    void cleanup();

public:
    StatisticsState(Game* gameInstance);
    ~StatisticsState();

    void handleInput(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);

    void onEnter();
    void onExit();
};
