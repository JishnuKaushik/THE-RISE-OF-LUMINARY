#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "StoryManager.hpp"

class Game;

class TrainingStoryState {
public:
    enum class Screen {
        SUBJECT_SELECT,
        CHAPTER_SELECT,
        READING_INTRO,
        READING_CONCEPT,
        READING_EXAMPLE,
        QUICK_CHECK,
        QUICK_RESULT
    };

private:
    Game* game;
    StoryManager storyManager;
    Screen screen;

    // Subject selection
    int selectedSubjectIndex;
    std::vector<std::string> subjects;
    std::vector<sf::RectangleShape*> subjectBoxes;
    std::vector<sf::Text*> subjectTexts;

    // Chapter selection (up to VISIBLE_CHAPTERS shown at once, scrollable)
    std::vector<StoryChapter> chapters;
    int selectedChapterIndex;
    int chapterScrollOffset;
    static constexpr int VISIBLE_CHAPTERS = 8;
    std::vector<sf::RectangleShape*> chapterBoxes;
    std::vector<sf::Text*> chapterTexts;

    // Index into chapters vector for the active reading/check session
    int activeChapterIndex;

    // Quick check
    int selectedAnswerIndex;
    bool answerSubmitted;
    bool answerCorrect;
    std::vector<sf::RectangleShape*> answerBoxes; // 4 boxes
    std::vector<sf::Text*> answerTexts;

    // Shared UI
    sf::Text* titleText;
    sf::Text* contentText;
    sf::Text* navText;
    sf::RectangleShape* contentPanel;
    sf::RectangleShape* bottomPanel;

    void loadSubjectChapters(const std::string& subject);
    void refreshChapterRows();
    void updateSubjectColors();
    void updateAnswerColors();
    bool isChapterCompleted(int chapterNum) const;
    void saveChapterCompletion(int chapterNum);
    void cleanup();

    static std::string wrapText(const std::string& text, const sf::Font& font,
                                unsigned int charSize, float maxWidth);

public:
    TrainingStoryState(Game* gameInstance);
    ~TrainingStoryState();

    void handleInput(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);

    void onEnter();
    void onExit();
};
