#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Game;

class TrainingStoryState {
private:
    Game* game;
    
    bool waitingForSubject;
    int selectedSubjectIndex;
    std::vector<std::string> subjects;
    std::vector<sf::RectangleShape*> subjectBoxes;
    std::vector<sf::Text*> subjectTexts;
    
    std::vector<std::string> currentChapters;
    int currentChapterIndex;
    int currentPart;
    
    sf::Text* titleText;
    sf::Text* contentText;
    sf::Text* continueText;
    sf::Text* backText;
    
    void loadStoryForSubject(const std::string& subject);
    void displayCurrentPart();
    void nextPart();
    void updateSubjectColors();
    void cleanup();
    
public:
    TrainingStoryState(Game* gameInstance);
    ~TrainingStoryState();
    
    void handleInput(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    
    void onEnter();
    void onExit();
};
