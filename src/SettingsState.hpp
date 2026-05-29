#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Game;

struct Settings {
    int musicVolume = 70;
    int sfxVolume   = 80;
    bool fullscreen = false;
};

class SettingsState {
private:
    Game* game;
    Settings settings;

    int selectedOption;  // 0=music, 1=sfx, 2=fullscreen

    sf::RectangleShape* bgPanel;
    sf::RectangleShape* optionPanels[3];
    sf::Text* titleText;
    sf::Text* optionTexts[3];
    sf::Text* navText;

    void updateOptionTexts();
    void saveSettings();
    void loadSettings();
    void cleanup();

public:
    SettingsState(Game* gameInstance);
    ~SettingsState();

    void handleInput(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);

    void onEnter();
    void onExit();
};
