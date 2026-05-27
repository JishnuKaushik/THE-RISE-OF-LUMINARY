#include "LessonViewerState.hpp"
#include "Game.hpp"
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <cctype>

namespace fs = std::filesystem;

// ─── Constructor ──────────────────────────────────────────────────────────────

LessonViewerState::LessonViewerState(Game* game)
    : game(game), currentPage(0), animTime(0.f),
      fadeDir(FadeDir::IN), fadeAlpha(255.f), targetPage(0)
{
    fadeRect.setSize(sf::Vector2f(1280.f, 720.f));
    fadeRect.setPosition(sf::Vector2f(0.f, 0.f));

    topBar.setSize(sf::Vector2f(1280.f, 52.f));
    topBar.setPosition(sf::Vector2f(0.f, 0.f));
    topBar.setFillColor(sf::Color(6, 4, 18, 218));

    bottomBar.setSize(sf::Vector2f(1280.f, 60.f));
    bottomBar.setPosition(sf::Vector2f(0.f, 660.f));
    bottomBar.setFillColor(sf::Color(6, 4, 18, 218));
}

// ─── loadPages ────────────────────────────────────────────────────────────────

void LessonViewerState::loadPages(const std::string& folderPath,
                                   const std::string& title) {
    textures.clear();
    sprites.clear();
    chapterTitle = title;

    std::error_code ec;
    if (!fs::exists(folderPath, ec) || !fs::is_directory(folderPath, ec)) {
        std::cerr << "[LessonViewer] Folder not found: " << folderPath << "\n";
        return;
    }

    // Collect image paths
    std::vector<std::string> paths;
    for (const auto& entry : fs::directory_iterator(folderPath, ec)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        for (char& c : ext) c = static_cast<char>(std::tolower((unsigned char)c));
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
            paths.push_back(entry.path().string());
    }
    std::sort(paths.begin(), paths.end());

    // Reserve so no reallocation invalidates sprite texture references
    textures.reserve(paths.size());
    for (const auto& p : paths) {
        sf::Texture tex;
        if (tex.loadFromFile(p))
            textures.push_back(std::move(tex));
        else
            std::cerr << "[LessonViewer] Cannot load: " << p << "\n";
    }

    // Build sprites after all textures are stable in memory
    sprites.reserve(textures.size());
    for (auto& tex : textures) {
        sprites.emplace_back(tex);
        fitSprite(sprites.back(), tex);
    }

    std::cout << "[LessonViewer] " << textures.size()
              << " pages loaded from " << folderPath << "\n";
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

void LessonViewerState::fitSprite(sf::Sprite& sprite, const sf::Texture& tex) const {
    sf::Vector2u ts = tex.getSize();
    float sx = 1280.f / ts.x;
    float sy = 720.f  / ts.y;
    float s  = std::min(sx, sy);
    sprite.setScale(sf::Vector2f(s, s));
    sprite.setPosition(sf::Vector2f(
        (1280.f - ts.x * s) * 0.5f,
        (720.f  - ts.y * s) * 0.5f));
}

void LessonViewerState::buildUI() {
    sf::Font& font = game->getMainFont();

    titleText     = std::make_unique<sf::Text>(font, chapterTitle, 20u);
    pageCountText = std::make_unique<sf::Text>(font, "", 18u);
    hintText      = std::make_unique<sf::Text>(font,
        "ENTER / \x1b   Next      BACKSPACE / \x1b   Prev      ESC   Skip to Practice",
        15u);

    titleText->setFillColor(sf::Color(255, 220, 100));
    titleText->setPosition(sf::Vector2f(20.f, 14.f));

    pageCountText->setFillColor(sf::Color(200, 190, 220));

    hintText->setFillColor(sf::Color(140, 130, 160, 200));
    {
        // Use simpler label without escape chars for the hint
        *hintText = sf::Text(font,
            "ENTER  Next        BACKSPACE  Prev        ESC  Skip to Practice", 15u);
        hintText->setFillColor(sf::Color(140, 130, 160, 200));
        auto hb = hintText->getLocalBounds();
        hintText->setPosition(sf::Vector2f((1280.f - hb.size.x) * 0.5f, 676.f));
    }
}

void LessonViewerState::applyPage(int page) {
    if (!pageCountText) return;
    int total = static_cast<int>(textures.size());
    if (total == 0) { pageCountText->setString(""); return; }
    pageCountText->setString(std::to_string(page + 1) + " / " + std::to_string(total));
    auto pb = pageCountText->getLocalBounds();
    pageCountText->setPosition(sf::Vector2f(1280.f - pb.size.x - 20.f, 16.f));
}

void LessonViewerState::triggerFade(int toPage) {
    targetPage = toPage;
    fadeDir    = FadeDir::OUT;
}

// ─── Lifecycle ────────────────────────────────────────────────────────────────

void LessonViewerState::onEnter() {
    currentPage = 0;
    animTime    = 0.f;
    fadeAlpha   = 255.f;
    fadeDir     = FadeDir::IN;
    targetPage  = 0;

    buildUI();
    applyPage(0);
    if (titleText)
        titleText->setString(chapterTitle);
}

void LessonViewerState::onExit() {}

// ─── Input ────────────────────────────────────────────────────────────────────

void LessonViewerState::handleInput(const sf::Event& event) {
    // Block all input while fading
    if (fadeDir != FadeDir::NONE) return;

    // Empty folder: any confirm key exits to practice
    if (textures.empty()) {
        if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
            if (kp->code == sf::Keyboard::Key::Enter ||
                kp->code == sf::Keyboard::Key::Escape)
                game->switchToTraining();
        return;
    }

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        switch (kp->code) {
            case sf::Keyboard::Key::Enter:
            case sf::Keyboard::Key::Right: {
                int next = currentPage + 1;
                triggerFade(next < (int)textures.size() ? next : -1);
                break;
            }
            case sf::Keyboard::Key::Backspace:
            case sf::Keyboard::Key::Left:
                if (currentPage > 0)
                    triggerFade(currentPage - 1);
                break;
            case sf::Keyboard::Key::Escape:
                triggerFade(-1);
                break;
            default: break;
        }
    }
}

// ─── Update ───────────────────────────────────────────────────────────────────

void LessonViewerState::update(float deltaTime) {
    animTime += deltaTime;

    if (fadeDir == FadeDir::IN) {
        fadeAlpha -= FADE_SPEED * deltaTime;
        if (fadeAlpha <= 0.f) { fadeAlpha = 0.f; fadeDir = FadeDir::NONE; }
    } else if (fadeDir == FadeDir::OUT) {
        fadeAlpha += FADE_SPEED * deltaTime;
        if (fadeAlpha >= 255.f) {
            fadeAlpha = 255.f;
            if (targetPage == -1) {
                game->switchToTraining();
                return;
            }
            currentPage = targetPage;
            applyPage(currentPage);
            fadeDir = FadeDir::IN;
        }
    }
}

// ─── Render ───────────────────────────────────────────────────────────────────

void LessonViewerState::render(sf::RenderWindow& window) {
    window.clear(sf::Color(8, 6, 20));

    // Page image
    if (!sprites.empty() && currentPage < (int)sprites.size()) {
        window.draw(sprites[currentPage]);
    } else if (textures.empty()) {
        if (pageCountText) {
            sf::Text msg(game->getMainFont(),
                "No lesson pages found.\nPress ENTER to start practice.", 26u);
            msg.setFillColor(sf::Color(200, 180, 220));
            auto mb = msg.getLocalBounds();
            msg.setPosition(sf::Vector2f((1280.f - mb.size.x) * 0.5f, 290.f));
            window.draw(msg);
        }
    }

    // Top bar
    window.draw(topBar);
    // Top bar gold bottom edge
    {
        sf::RectangleShape edge(sf::Vector2f(1280.f, 1.5f));
        edge.setPosition(sf::Vector2f(0.f, 51.f));
        edge.setFillColor(sf::Color(180, 140, 40, 160));
        window.draw(edge);
    }

    // Bottom bar
    // Gold top edge
    {
        sf::RectangleShape edge(sf::Vector2f(1280.f, 1.5f));
        edge.setPosition(sf::Vector2f(0.f, 660.f));
        edge.setFillColor(sf::Color(180, 140, 40, 160));
        window.draw(edge);
    }
    window.draw(bottomBar);

    // UI text in bars
    if (titleText)     window.draw(*titleText);
    if (pageCountText) window.draw(*pageCountText);
    if (hintText)      window.draw(*hintText);

    // Pulsing "Press ENTER to continue" just above the bottom bar
    if (!textures.empty() && fadeDir == FadeDir::NONE) {
        float a = 150.f + std::sin(animTime * 2.5f) * 80.f;
        sf::Text cont(game->getMainFont(), "Press ENTER to continue", 18u);
        cont.setFillColor(sf::Color(220, 210, 180, static_cast<uint8_t>(a)));
        auto cb = cont.getLocalBounds();
        cont.setPosition(sf::Vector2f((1280.f - cb.size.x) * 0.5f, 630.f));
        window.draw(cont);
    }

    // Fade overlay — drawn last so it covers everything
    fadeRect.setFillColor(sf::Color(0, 0, 0, static_cast<uint8_t>(fadeAlpha)));
    window.draw(fadeRect);
}
