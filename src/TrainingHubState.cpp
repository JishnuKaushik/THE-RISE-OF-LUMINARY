#include "TrainingHubState.hpp"
#include "Game.hpp"
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <cctype>

namespace fs = std::filesystem;

// ─── Static data ──────────────────────────────────────────────────────────────

const std::vector<std::string> TrainingHubState::SUBJECTS = {
    "Mathematics", "Science", "History", "Literature", "Arts"
};

// ─── UI helpers (local to this TU) ───────────────────────────────────────────

static void hub_drawGlassPanel(sf::RenderTarget& target, sf::FloatRect b,
                                float glow,
                                sf::Color bg = sf::Color(15, 12, 40, 145)) {
    auto clamp = [](int v) -> uint8_t {
        return static_cast<uint8_t>(v < 0 ? 0 : (v > 255 ? 255 : v));
    };

    // Drop shadow
    sf::RectangleShape shadow(b.size + sf::Vector2f(8.f, 8.f));
    shadow.setPosition(b.position + sf::Vector2f(4.f, 5.f));
    shadow.setFillColor(sf::Color(0, 0, 0, 65));
    target.draw(shadow);

    // Glass fill
    sf::RectangleShape panel(b.size);
    panel.setPosition(b.position);
    panel.setFillColor(bg);
    target.draw(panel);

    // Top-edge refraction highlight
    sf::RectangleShape hi(sf::Vector2f(b.size.x * 0.86f, 1.5f));
    hi.setPosition(sf::Vector2f(b.position.x + b.size.x * 0.07f, b.position.y + 3.f));
    hi.setFillColor(sf::Color(210, 190, 255, 30));
    target.draw(hi);

    // Animated gold border
    sf::Color border(
        clamp(165 + (int)(glow * 90)),
        clamp(125 + (int)(glow * 30)),
        40,
        clamp(150 + (int)(glow * 105))
    );
    const float t  = 1.5f;
    const float cx = b.position.x, cy = b.position.y;
    const float cw = b.size.x,    ch = b.size.y;
    sf::RectangleShape r;
    r.setFillColor(border);
    r.setSize({cw, t});  r.setPosition({cx,       cy});          target.draw(r);
                          r.setPosition({cx,       cy + ch - t}); target.draw(r);
    r.setSize({t, ch});  r.setPosition({cx,       cy});           target.draw(r);
                          r.setPosition({cx + cw - t, cy});       target.draw(r);

    // Corner accents
    const float cs = 7.f;
    r.setSize({cs, cs});
    r.setFillColor(sf::Color(255, 220, 80, clamp(155 + (int)(glow * 100))));
    r.setPosition({cx,       cy});        target.draw(r);
    r.setPosition({cx+cw-cs, cy});        target.draw(r);
    r.setPosition({cx,       cy+ch-cs});  target.draw(r);
    r.setPosition({cx+cw-cs, cy+ch-cs});  target.draw(r);
}

static void hub_shadow(sf::RenderTarget& target, sf::Text& text, sf::Vector2f pos) {
    sf::Color c = text.getFillColor();
    text.setPosition(pos + sf::Vector2f(2.f, 2.f));
    text.setFillColor(sf::Color(0, 0, 0, 160));
    target.draw(text);
    text.setPosition(pos);
    text.setFillColor(c);
    target.draw(text);
}

static void hub_vignette(sf::RenderTarget& target) {
    constexpr float W = 1280.f, H = 720.f, E = 80.f;
    sf::RectangleShape v;
    v.setFillColor(sf::Color(0, 0, 0, 110));
    v.setSize({W, E});   v.setPosition({0.f, 0.f});      target.draw(v);
                          v.setPosition({0.f, H - E});    target.draw(v);
    v.setSize({E, H});   v.setPosition({0.f, 0.f});      target.draw(v);
                          v.setPosition({W - E, 0.f});    target.draw(v);
}

// ─── Static helpers ───────────────────────────────────────────────────────────

std::string TrainingHubState::subjectToFolder(const std::string& s) {
    if (s == "Mathematics") return "math";
    if (s == "Science")     return "science";
    if (s == "History")     return "history";
    if (s == "Literature")  return "literature";
    if (s == "Arts")        return "arts";
    return "";
}

std::string TrainingHubState::folderToDisplayName(const std::string& folder) {
    std::string out;
    bool cap = true;
    for (char c : folder) {
        if (c == '_' || c == '-') { out += ' '; cap = true; }
        else if (cap) { out += static_cast<char>(std::toupper((unsigned char)c)); cap = false; }
        else          { out += c; }
    }
    return out;
}

// ─── Constructor ──────────────────────────────────────────────────────────────

TrainingHubState::TrainingHubState(Game* game)
    : game(game), currentScreen(Screen::SUBJECT),
      animTime(0.f), fadeAlpha(255.f), fadingIn(true),
      selectedSubjectIdx(0), selectedChapterIdx(0)
{
    if (bgTexture.loadFromFile("assets/images/menu.png")) {
        bgSprite.emplace(bgTexture);
        bgSprite->setScale(sf::Vector2f(
            1280.f / bgTexture.getSize().x,
            720.f  / bgTexture.getSize().y));
    }
}

// ─── Lifecycle ────────────────────────────────────────────────────────────────

void TrainingHubState::onEnter() {
    currentScreen      = Screen::SUBJECT;
    selectedSubjectIdx = 0;
    selectedChapterIdx = 0;
    chapterNames.clear();
    chapterPaths.clear();
    animTime  = 0.f;
    fadeAlpha = 255.f;
    fadingIn  = true;
}

void TrainingHubState::onExit() {}

// ─── Chapter scanning ─────────────────────────────────────────────────────────

void TrainingHubState::scanChapters() {
    chapterNames.clear();
    chapterPaths.clear();

    std::string folder = subjectToFolder(SUBJECTS[selectedSubjectIdx]);
    if (folder.empty()) return;

    std::string base = "assets/training_story/" + folder;

    std::error_code ec;
    if (!fs::exists(base, ec) || !fs::is_directory(base, ec)) {
        std::cout << "[TrainingHub] No folder: " << base << "\n";
        return;
    }

    std::vector<std::pair<std::string, std::string>> entries;
    for (const auto& e : fs::directory_iterator(base, ec)) {
        if (e.is_directory())
            entries.push_back({e.path().filename().string(), e.path().string()});
    }
    std::sort(entries.begin(), entries.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    for (auto& [name, path] : entries) {
        chapterNames.push_back(folderToDisplayName(name));
        chapterPaths.push_back(path);
    }
    std::cout << "[TrainingHub] " << chapterNames.size()
              << " chapters for " << SUBJECTS[selectedSubjectIdx] << "\n";
}

// ─── Confirm actions ──────────────────────────────────────────────────────────

void TrainingHubState::confirmSubject() {
    game->selectedSubject = SUBJECTS[selectedSubjectIdx];
    scanChapters();
    currentScreen      = Screen::CHAPTER;
    selectedChapterIdx = 0;
    // Reset fade for screen transition feel
    fadeAlpha = 80.f;
    fadingIn  = true;
}

void TrainingHubState::confirmChapter() {
    game->selectedSubject = SUBJECTS[selectedSubjectIdx];
    if (chapterPaths.empty()) {
        // No lesson pages — go straight to practice
        game->switchToTraining();
        return;
    }
    game->trainingChapterPath  = chapterPaths[selectedChapterIdx];
    game->trainingChapterTitle = chapterNames[selectedChapterIdx];
    game->switchToLessonViewer();
}

// ─── Mouse helpers ────────────────────────────────────────────────────────────

static float cardStartY(int count) {
    float totalH = count * TrainingHubState::CARD_H + (count - 1) * TrainingHubState::CARD_GAP;
    return (720.f - totalH) * 0.5f + 20.f;
}

void TrainingHubState::handleMouseMove(sf::Vector2f mp) {
    if (currentScreen == Screen::SUBJECT) {
        const int n = (int)SUBJECTS.size();
        float sy = cardStartY(n);
        for (int i = 0; i < n; i++) {
            float cy = sy + i * (CARD_H + CARD_GAP);
            if (sf::FloatRect({CARD_X, cy}, {CARD_W, CARD_H}).contains(mp))
                selectedSubjectIdx = i;
        }
    } else {
        const int n = (int)chapterNames.size();
        if (n == 0) return;
        float sy = cardStartY(n);
        for (int i = 0; i < n; i++) {
            float cy = sy + i * (CARD_H + CARD_GAP);
            if (sf::FloatRect({CARD_X, cy}, {CARD_W, CARD_H}).contains(mp))
                selectedChapterIdx = i;
        }
    }
}

void TrainingHubState::handleMouseClick(sf::Vector2f mp) {
    if (currentScreen == Screen::SUBJECT) {
        const int n = (int)SUBJECTS.size();
        float sy = cardStartY(n);
        for (int i = 0; i < n; i++) {
            float cy = sy + i * (CARD_H + CARD_GAP);
            if (sf::FloatRect({CARD_X, cy}, {CARD_W, CARD_H}).contains(mp)) {
                selectedSubjectIdx = i;
                confirmSubject();
                return;
            }
        }
    } else {
        if (chapterNames.empty()) { confirmChapter(); return; }
        const int n = (int)chapterNames.size();
        float sy = cardStartY(n);
        for (int i = 0; i < n; i++) {
            float cy = sy + i * (CARD_H + CARD_GAP);
            if (sf::FloatRect({CARD_X, cy}, {CARD_W, CARD_H}).contains(mp)) {
                selectedChapterIdx = i;
                confirmChapter();
                return;
            }
        }
    }
}

// ─── Input ────────────────────────────────────────────────────────────────────

void TrainingHubState::handleInput(const sf::Event& event) {
    if (const auto* mm = event.getIf<sf::Event::MouseMoved>())
        handleMouseMove({(float)mm->position.x, (float)mm->position.y});

    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>())
        if (mb->button == sf::Mouse::Button::Left)
            handleMouseClick({(float)mb->position.x, (float)mb->position.y});

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        if (currentScreen == Screen::SUBJECT) {
            const int n = (int)SUBJECTS.size();
            switch (kp->code) {
                case sf::Keyboard::Key::Up:
                case sf::Keyboard::Key::W:
                    selectedSubjectIdx = (selectedSubjectIdx - 1 + n) % n; break;
                case sf::Keyboard::Key::Down:
                case sf::Keyboard::Key::S:
                    selectedSubjectIdx = (selectedSubjectIdx + 1) % n; break;
                case sf::Keyboard::Key::Enter:
                    confirmSubject(); break;
                case sf::Keyboard::Key::Escape:
                    game->switchToMenu(); break;
                default: break;
            }
        } else { // CHAPTER
            const int n = (int)(chapterNames.empty() ? 1 : chapterNames.size());
            switch (kp->code) {
                case sf::Keyboard::Key::Up:
                case sf::Keyboard::Key::W:
                    if (!chapterNames.empty())
                        selectedChapterIdx = (selectedChapterIdx - 1 + n) % n;
                    break;
                case sf::Keyboard::Key::Down:
                case sf::Keyboard::Key::S:
                    if (!chapterNames.empty())
                        selectedChapterIdx = (selectedChapterIdx + 1) % n;
                    break;
                case sf::Keyboard::Key::Enter:
                    confirmChapter(); break;
                case sf::Keyboard::Key::Escape:
                    currentScreen      = Screen::SUBJECT;
                    selectedChapterIdx = 0;
                    fadeAlpha = 80.f;
                    fadingIn  = true;
                    break;
                default: break;
            }
        }
    }
}

// ─── Update ───────────────────────────────────────────────────────────────────

void TrainingHubState::update(float deltaTime) {
    animTime += deltaTime;
    if (fadingIn) {
        fadeAlpha -= deltaTime * 510.f;
        if (fadeAlpha <= 0.f) { fadeAlpha = 0.f; fadingIn = false; }
    }
}

// ─── Render helpers ───────────────────────────────────────────────────────────

void TrainingHubState::renderSubjectScreen(sf::RenderWindow& window) {
    sf::Font& font = game->getMainFont();
    const float pulse = (std::sin(animTime * 1.8f) + 1.f) * 0.5f;

    // Title
    {
        sf::Text title(font, "TRAINING  HUB", 48);
        title.setFillColor(sf::Color(255, 230, 100));
        auto b = title.getLocalBounds();
        sf::Vector2f pos((1280.f - b.size.x) * 0.5f, 54.f);
        hub_shadow(window, title, pos);
    }
    {
        sf::Text sub(font, "Choose Your Subject", 22);
        sub.setFillColor(sf::Color(180, 160, 220, 200));
        auto b = sub.getLocalBounds();
        sf::Vector2f pos((1280.f - b.size.x) * 0.5f, 118.f);
        hub_shadow(window, sub, pos);
    }

    // Cards
    const int n = (int)SUBJECTS.size();
    float sy = cardStartY(n);
    for (int i = 0; i < n; i++) {
        float cy = sy + i * (CARD_H + CARD_GAP);
        bool sel = (i == selectedSubjectIdx);
        float glow = sel ? (0.5f + pulse * 0.5f) : 0.05f;
        sf::Color bg = sel ? sf::Color(30, 20, 70, 200) : sf::Color(12, 9, 35, 160);

        hub_drawGlassPanel(window, {{CARD_X, cy}, {CARD_W, CARD_H}}, glow, bg);

        if (sel) {
            sf::RectangleShape bar(sf::Vector2f(4.f, CARD_H - 16.f));
            bar.setPosition({CARD_X + 8.f, cy + 8.f});
            bar.setFillColor(sf::Color(255, 200, 60,
                static_cast<uint8_t>(180 + (int)(pulse * 75))));
            window.draw(bar);
        }

        sf::Text label(font, SUBJECTS[i], sel ? 26u : 24u);
        label.setFillColor(sel ? sf::Color(255, 230, 100) : sf::Color(200, 190, 220));
        auto lb = label.getLocalBounds();
        sf::Vector2f lp(CARD_X + (CARD_W - lb.size.x) * 0.5f,
                        cy + (CARD_H - lb.size.y) * 0.5f - 4.f);
        hub_shadow(window, label, lp);
    }

    // Bottom hint
    sf::Text hint(font, "W / UP  Navigate        ENTER  Select        ESC  Menu", 16);
    hint.setFillColor(sf::Color(140, 130, 160, 180));
    auto hb = hint.getLocalBounds();
    hint.setPosition({(1280.f - hb.size.x) * 0.5f, 672.f});
    window.draw(hint);
}

void TrainingHubState::renderChapterScreen(sf::RenderWindow& window) {
    sf::Font& font = game->getMainFont();
    const float pulse = (std::sin(animTime * 1.8f) + 1.f) * 0.5f;

    // Title
    {
        std::string ts = SUBJECTS[selectedSubjectIdx] + "   —   Choose Chapter";
        sf::Text title(font, ts, 34);
        title.setFillColor(sf::Color(255, 230, 100));
        auto b = title.getLocalBounds();
        sf::Vector2f pos((1280.f - b.size.x) * 0.5f, 54.f);
        hub_shadow(window, title, pos);
    }

    if (chapterNames.empty()) {
        sf::Text msg(font, "No lesson pages found for this subject yet.", 24);
        msg.setFillColor(sf::Color(200, 180, 220));
        auto mb = msg.getLocalBounds();
        sf::Vector2f mp((1280.f - mb.size.x) * 0.5f, 300.f);
        hub_shadow(window, msg, mp);

        sf::Text sub(font, "Press ENTER to go directly to practice questions.", 20);
        sub.setFillColor(sf::Color(160, 150, 180));
        auto sb = sub.getLocalBounds();
        sf::Vector2f sp((1280.f - sb.size.x) * 0.5f, 355.f);
        hub_shadow(window, sub, sp);
    } else {
        const int n = (int)chapterNames.size();
        float sy = cardStartY(n);
        for (int i = 0; i < n; i++) {
            float cy = sy + i * (CARD_H + CARD_GAP);
            bool sel = (i == selectedChapterIdx);
            float glow = sel ? (0.5f + pulse * 0.5f) : 0.05f;
            sf::Color bg = sel ? sf::Color(30, 20, 70, 200) : sf::Color(12, 9, 35, 160);

            hub_drawGlassPanel(window, {{CARD_X, cy}, {CARD_W, CARD_H}}, glow, bg);

            if (sel) {
                sf::RectangleShape bar(sf::Vector2f(4.f, CARD_H - 16.f));
                bar.setPosition({CARD_X + 8.f, cy + 8.f});
                bar.setFillColor(sf::Color(255, 200, 60,
                    static_cast<uint8_t>(180 + (int)(pulse * 75))));
                window.draw(bar);
            }

            sf::Text label(font, chapterNames[i], sel ? 24u : 22u);
            label.setFillColor(sel ? sf::Color(255, 230, 100) : sf::Color(200, 190, 220));
            auto lb = label.getLocalBounds();
            sf::Vector2f lp(CARD_X + (CARD_W - lb.size.x) * 0.5f,
                            cy + (CARD_H - lb.size.y) * 0.5f - 4.f);
            hub_shadow(window, label, lp);
        }
    }

    // Bottom hint
    sf::Text hint(font, "W / UP  Navigate        ENTER  Select        ESC  Back", 16);
    hint.setFillColor(sf::Color(140, 130, 160, 180));
    auto hb = hint.getLocalBounds();
    hint.setPosition({(1280.f - hb.size.x) * 0.5f, 672.f});
    window.draw(hint);
}

// ─── Render ───────────────────────────────────────────────────────────────────

void TrainingHubState::render(sf::RenderWindow& window) {
    if (bgSprite.has_value())
        window.draw(*bgSprite);

    // Dark overlay for readability over background image
    sf::RectangleShape overlay(sf::Vector2f(1280.f, 720.f));
    overlay.setFillColor(sf::Color(0, 0, 0, 130));
    window.draw(overlay);

    hub_vignette(window);

    if (currentScreen == Screen::SUBJECT)
        renderSubjectScreen(window);
    else
        renderChapterScreen(window);

    // Fade-in overlay
    if (fadeAlpha > 0.f) {
        sf::RectangleShape fade(sf::Vector2f(1280.f, 720.f));
        fade.setFillColor(sf::Color(0, 0, 0, static_cast<uint8_t>(fadeAlpha)));
        window.draw(fade);
    }
}
