#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <optional>

class Game;

class TrainingHubState {
public:
    TrainingHubState(Game* game);
    ~TrainingHubState() = default;

    void handleInput(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);

    void onEnter();
    void onExit();

private:
    enum class Screen { SUBJECT, CHAPTER };

    Game*  game;
    Screen currentScreen;
    float  animTime;
    float  fadeAlpha;
    bool   fadingIn;

    // Subject screen
    static const std::vector<std::string> SUBJECTS;
    int selectedSubjectIdx;

    // Chapter screen
    std::vector<std::string> chapterNames;
    std::vector<std::string> chapterPaths;
    int selectedChapterIdx;

    // Background
    sf::Texture              bgTexture;
    std::optional<sf::Sprite> bgSprite;

    void scanChapters();
    void confirmSubject();
    void confirmChapter();
    void handleMouseMove(sf::Vector2f mp);
    void handleMouseClick(sf::Vector2f mp);

    void renderSubjectScreen(sf::RenderWindow& window);
    void renderChapterScreen(sf::RenderWindow& window);

    static std::string subjectToFolder(const std::string& subject);
    static std::string folderToDisplayName(const std::string& folder);

public:
    // Shared card layout constants (public so free helpers in .cpp can use them)
    static constexpr float CARD_W = 520.f;
    static constexpr float CARD_H = 68.f;
    static constexpr float CARD_GAP = 12.f;
    static constexpr float CARD_X = (1280.f - CARD_W) * 0.5f;
};
