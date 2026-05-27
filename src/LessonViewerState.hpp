#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>

class Game;

class LessonViewerState {
public:
    explicit LessonViewerState(Game* game);
    ~LessonViewerState() = default;

    // Call this before onEnter() to supply page images
    void loadPages(const std::string& folderPath, const std::string& chapterTitle);

    void handleInput(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);

    void onEnter();
    void onExit();

private:
    Game* game;

    // Page data — textures must outlive sprites, reserve() keeps addresses stable
    std::vector<sf::Texture> textures;
    std::vector<sf::Sprite>  sprites;
    int  currentPage;
    std::string chapterTitle;

    float animTime;

    // Fade transition state machine
    enum class FadeDir { NONE, OUT, IN };
    FadeDir fadeDir;
    float   fadeAlpha;  // 0 = clear, 255 = black
    int     targetPage; // -1 = exit to TrainingState

    // Persistent drawables
    sf::RectangleShape fadeRect;
    sf::RectangleShape topBar;
    sf::RectangleShape bottomBar;

    // Text — created in onEnter() when font is ready
    std::unique_ptr<sf::Text> titleText;
    std::unique_ptr<sf::Text> pageCountText;
    std::unique_ptr<sf::Text> hintText;

    static constexpr float FADE_SPEED = 560.f; // alpha units/sec

    void triggerFade(int toPage);
    void applyPage(int page);
    void fitSprite(sf::Sprite& sprite, const sf::Texture& tex) const;
    void buildUI();
};
