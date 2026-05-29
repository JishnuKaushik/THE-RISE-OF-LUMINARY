#include "TrainingStoryState.hpp"
#include "Game.hpp"
#include "SaveManager.hpp"
#include <iostream>
#include <algorithm>

// ─── Static helpers ───────────────────────────────────────────────────────────

std::string TrainingStoryState::wrapText(const std::string& text, const sf::Font& font,
                                          unsigned int charSize, float maxWidth) {
    std::string result, currentLine, word;
    auto flushWord = [&]() {
        if (word.empty()) return;
        std::string test = currentLine.empty() ? word : currentLine + " " + word;
        sf::Text probe(font, test, charSize);
        if (!currentLine.empty() && probe.getLocalBounds().size.x > maxWidth) {
            if (!result.empty()) result += '\n';
            result += currentLine;
            currentLine = word;
        } else {
            currentLine = test;
        }
        word.clear();
    };
    for (char c : text) {
        if      (c == ' ')  { flushWord(); }
        else if (c == '\n') {
            flushWord();
            if (!result.empty()) result += '\n';
            result += currentLine;
            currentLine.clear();
        } else {
            word += c;
        }
    }
    flushWord();
    if (!currentLine.empty()) { if (!result.empty()) result += '\n'; result += currentLine; }
    return result;
}

static void drawStoryPanel(sf::RenderTarget& target, sf::FloatRect b,
                            sf::Color fill = sf::Color(14, 10, 38, 145)) {
    sf::RectangleShape shadow(b.size + sf::Vector2f(6.f, 6.f));
    shadow.setPosition(b.position + sf::Vector2f(3.f, 4.f));
    shadow.setFillColor(sf::Color(0, 0, 0, 60));
    target.draw(shadow);

    sf::RectangleShape panel(b.size);
    panel.setPosition(b.position);
    panel.setFillColor(fill);
    panel.setOutlineColor(sf::Color(165, 125, 40, 150));
    panel.setOutlineThickness(1.5f);
    target.draw(panel);

    // Corner accents
    const float cs = 7.f;
    sf::RectangleShape corner(sf::Vector2f(cs, cs));
    corner.setFillColor(sf::Color(255, 220, 80, 155));
    corner.setPosition(sf::Vector2f(b.position.x,                 b.position.y));             target.draw(corner);
    corner.setPosition(sf::Vector2f(b.position.x + b.size.x - cs, b.position.y));             target.draw(corner);
    corner.setPosition(sf::Vector2f(b.position.x,                 b.position.y + b.size.y - cs)); target.draw(corner);
    corner.setPosition(sf::Vector2f(b.position.x + b.size.x - cs, b.position.y + b.size.y - cs)); target.draw(corner);
}

static void drawStoryText(sf::RenderTarget& target, sf::Text& text, sf::Vector2f pos) {
    sf::Color main = text.getFillColor();
    text.setPosition(pos + sf::Vector2f(2.f, 2.f));
    text.setFillColor(sf::Color(0, 0, 0, 150));
    target.draw(text);
    text.setPosition(pos);
    text.setFillColor(main);
    target.draw(text);
}

// ─────────────────────────────────────────────────────────────────────────────

TrainingStoryState::TrainingStoryState(Game* gameInstance)
    : game(gameInstance),
      screen(Screen::SUBJECT_SELECT),
      selectedSubjectIndex(0),
      selectedChapterIndex(0),
      chapterScrollOffset(0),
      activeChapterIndex(0),
      selectedAnswerIndex(0),
      answerSubmitted(false),
      answerCorrect(false)
{
    sf::Font& font = game->getMainFont();
    subjects = { "Mathematics", "Science", "History", "Literature", "Arts" };

    // ── Shared UI ─────────────────────────────────────────────────────────────
    titleText = new sf::Text(font, "STORY MODE", 34);
    titleText->setFillColor(sf::Color(255, 220, 80));
    titleText->setPosition(sf::Vector2f(640.f, 28.f));

    contentText = new sf::Text(font, "", 20);
    contentText->setFillColor(sf::Color(225, 225, 225));
    contentText->setPosition(sf::Vector2f(90.f, 118.f));

    navText = new sf::Text(font, "", 17);
    navText->setFillColor(sf::Color(180, 180, 180));
    navText->setPosition(sf::Vector2f(30.f, 681.f));

    contentPanel = new sf::RectangleShape(sf::Vector2f(1160.f, 530.f));
    contentPanel->setPosition(sf::Vector2f(60.f, 92.f));

    bottomPanel = new sf::RectangleShape(sf::Vector2f(1280.f, 55.f));
    bottomPanel->setPosition(sf::Vector2f(0.f, 662.f));

    // ── Subject selection (5 boxes, centred vertically) ───────────────────────
    const float subW = 420.f, subH = 52.f, subGap = 64.f;
    const float subX = 430.f, subStartY = 170.f;
    for (int i = 0; i < 5; i++) {
        auto* box = new sf::RectangleShape(sf::Vector2f(subW, subH));
        box->setPosition(sf::Vector2f(subX, subStartY + i * subGap));
        box->setFillColor(sf::Color(30, 15, 55, 200));
        box->setOutlineColor(sf::Color(150, 100, 50, 180));
        box->setOutlineThickness(2.f);
        subjectBoxes.push_back(box);

        auto* text = new sf::Text(font, subjects[i], 22);
        text->setFillColor(sf::Color::White);
        text->setPosition(sf::Vector2f(subX + 20.f, subStartY + i * subGap + 12.f));
        subjectTexts.push_back(text);
    }

    // ── Chapter list (8 visible rows) ─────────────────────────────────────────
    const float chapW = 1100.f, chapH = 50.f, chapGap = 58.f;
    const float chapX = 90.f, chapStartY = 108.f;
    for (int i = 0; i < VISIBLE_CHAPTERS; i++) {
        auto* box = new sf::RectangleShape(sf::Vector2f(chapW, chapH));
        box->setPosition(sf::Vector2f(chapX, chapStartY + i * chapGap));
        box->setFillColor(sf::Color(30, 15, 55, 180));
        box->setOutlineColor(sf::Color(150, 100, 50, 150));
        box->setOutlineThickness(1.5f);
        chapterBoxes.push_back(box);

        auto* text = new sf::Text(font, "", 19);
        text->setFillColor(sf::Color(200, 200, 200));
        text->setPosition(sf::Vector2f(chapX + 16.f, chapStartY + i * chapGap + 13.f));
        chapterTexts.push_back(text);
    }

    // ── Quick-check answer boxes (2×2 grid) ───────────────────────────────────
    const float ansW = 530.f, ansH = 72.f;
    const float ansXY[4][2] = {
        { 90.f, 285.f }, { 660.f, 285.f },
        { 90.f, 368.f }, { 660.f, 368.f }
    };
    for (int i = 0; i < 4; i++) {
        auto* box = new sf::RectangleShape(sf::Vector2f(ansW, ansH));
        box->setPosition(sf::Vector2f(ansXY[i][0], ansXY[i][1]));
        box->setFillColor(sf::Color(30, 15, 55, 185));
        box->setOutlineColor(sf::Color(200, 200, 200, 160));
        box->setOutlineThickness(2.f);
        answerBoxes.push_back(box);

        auto* text = new sf::Text(font, "", 19);
        text->setFillColor(sf::Color(200, 200, 200));
        text->setPosition(sf::Vector2f(ansXY[i][0] + 14.f, ansXY[i][1] + 22.f));
        answerTexts.push_back(text);
    }

    updateSubjectColors();
    std::cout << "TrainingStoryState created\n";
}

TrainingStoryState::~TrainingStoryState() {
    cleanup();
}

void TrainingStoryState::cleanup() {
    delete titleText;
    delete contentText;
    delete navText;
    delete contentPanel;
    delete bottomPanel;
    for (auto* b : subjectBoxes)  delete b;
    for (auto* t : subjectTexts)  delete t;
    for (auto* b : chapterBoxes)  delete b;
    for (auto* t : chapterTexts)  delete t;
    for (auto* b : answerBoxes)   delete b;
    for (auto* t : answerTexts)   delete t;
    subjectBoxes.clear(); subjectTexts.clear();
    chapterBoxes.clear(); chapterTexts.clear();
    answerBoxes.clear();  answerTexts.clear();
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

bool TrainingStoryState::isChapterCompleted(int chapterNum) const {
    const auto& v = game->playerData.completedChapters;
    return std::find(v.begin(), v.end(), chapterNum) != v.end();
}

void TrainingStoryState::saveChapterCompletion(int chapterNum) {
    if (!isChapterCompleted(chapterNum)) {
        game->playerData.completedChapters.push_back(chapterNum);
        game->playerData.dailyChaptersCompleted++;
        SaveManager::savePlayer(game->playerData);
        game->achievements.checkStoryChapter();
    }
}

void TrainingStoryState::loadSubjectChapters(const std::string& subject) {
    chapters             = storyManager.getStoriesBySubject(subject);
    selectedChapterIndex = 0;
    chapterScrollOffset  = 0;
    refreshChapterRows();
}

void TrainingStoryState::refreshChapterRows() {
    int total = static_cast<int>(chapters.size());
    for (int i = 0; i < VISIBLE_CHAPTERS; i++) {
        int idx = chapterScrollOffset + i;
        if (idx < total) {
            const auto& ch = chapters[idx];
            std::string label = "Ch " + std::to_string(ch.chapterNumber) + ":  " + ch.chapterName;
            if (isChapterCompleted(ch.chapterNumber)) label += "   [DONE]";
            chapterTexts[i]->setString(label);
            chapterBoxes[i]->setFillColor(sf::Color(30, 15, 55, 180));
            chapterBoxes[i]->setOutlineThickness(1.5f);
        } else {
            chapterTexts[i]->setString("");
            chapterBoxes[i]->setFillColor(sf::Color(0, 0, 0, 0));
            chapterBoxes[i]->setOutlineThickness(0.f);
        }
    }
    int selVisible = selectedChapterIndex - chapterScrollOffset;
    for (int i = 0; i < VISIBLE_CHAPTERS; i++) {
        if (i == selVisible) {
            chapterBoxes[i]->setOutlineColor(sf::Color(255, 200, 0, 220));
            chapterBoxes[i]->setOutlineThickness(3.f);
            chapterTexts[i]->setFillColor(sf::Color(255, 220, 80));
        } else {
            chapterBoxes[i]->setOutlineColor(sf::Color(150, 100, 50, 150));
            chapterTexts[i]->setFillColor(sf::Color(200, 200, 200));
        }
    }
}

void TrainingStoryState::updateSubjectColors() {
    for (int i = 0; i < (int)subjectBoxes.size(); i++) {
        if (i == selectedSubjectIndex) {
            subjectBoxes[i]->setOutlineColor(sf::Color(255, 200, 0, 230));
            subjectBoxes[i]->setOutlineThickness(3.f);
            subjectTexts[i]->setFillColor(sf::Color(255, 220, 80));
        } else {
            subjectBoxes[i]->setOutlineColor(sf::Color(150, 100, 50, 160));
            subjectBoxes[i]->setOutlineThickness(2.f);
            subjectTexts[i]->setFillColor(sf::Color(200, 200, 200));
        }
    }
}

void TrainingStoryState::updateAnswerColors() {
    if (chapters.empty() || activeChapterIndex >= (int)chapters.size()) return;
    const auto& ch = chapters[activeChapterIndex];
    for (int i = 0; i < 4; i++) {
        if (!answerSubmitted) {
            if (i == selectedAnswerIndex) {
                answerBoxes[i]->setOutlineColor(sf::Color(255, 200, 0, 230));
                answerBoxes[i]->setOutlineThickness(3.f);
                answerTexts[i]->setFillColor(sf::Color(255, 220, 80));
            } else {
                answerBoxes[i]->setOutlineColor(sf::Color(200, 200, 200, 150));
                answerBoxes[i]->setOutlineThickness(2.f);
                answerTexts[i]->setFillColor(sf::Color(200, 200, 200));
            }
        } else {
            if (i == ch.quickCheck.correct) {
                answerBoxes[i]->setOutlineColor(sf::Color(60, 220, 100));
                answerBoxes[i]->setOutlineThickness(3.f);
                answerTexts[i]->setFillColor(sf::Color(100, 255, 140));
            } else if (i == selectedAnswerIndex) {
                answerBoxes[i]->setOutlineColor(sf::Color(220, 60, 60));
                answerBoxes[i]->setOutlineThickness(3.f);
                answerTexts[i]->setFillColor(sf::Color(255, 100, 100));
            } else {
                answerBoxes[i]->setOutlineColor(sf::Color(90, 90, 90, 100));
                answerBoxes[i]->setOutlineThickness(1.f);
                answerTexts[i]->setFillColor(sf::Color(140, 140, 140));
            }
        }
    }
}

// ─── Input ───────────────────────────────────────────────────────────────────

void TrainingStoryState::handleInput(const sf::Event& event) {
    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        sf::Font& font = game->getMainFont();

        switch (screen) {

        // ── Subject selection ─────────────────────────────────────────────────
        case Screen::SUBJECT_SELECT:
            if (kp->code == sf::Keyboard::Key::Up) {
                selectedSubjectIndex = (selectedSubjectIndex - 1 + 5) % 5;
                updateSubjectColors();
            } else if (kp->code == sf::Keyboard::Key::Down) {
                selectedSubjectIndex = (selectedSubjectIndex + 1) % 5;
                updateSubjectColors();
            } else if (kp->code == sf::Keyboard::Key::Enter) {
                game->selectedSubject = subjects[selectedSubjectIndex];
                loadSubjectChapters(game->selectedSubject);
                screen = Screen::CHAPTER_SELECT;
                titleText->setString(game->selectedSubject + " - Chapters");
                navText->setString("UP/DOWN: Navigate   ENTER: Read   ESC: Back");
            } else if (kp->code == sf::Keyboard::Key::Escape) {
                game->switchToMenu();
            }
            break;

        // ── Chapter selection ─────────────────────────────────────────────────
        case Screen::CHAPTER_SELECT: {
            int total = static_cast<int>(chapters.size());
            if (kp->code == sf::Keyboard::Key::Up && selectedChapterIndex > 0) {
                selectedChapterIndex--;
                if (selectedChapterIndex < chapterScrollOffset)
                    chapterScrollOffset = selectedChapterIndex;
                refreshChapterRows();
            } else if (kp->code == sf::Keyboard::Key::Down
                       && selectedChapterIndex < total - 1) {
                selectedChapterIndex++;
                if (selectedChapterIndex >= chapterScrollOffset + VISIBLE_CHAPTERS)
                    chapterScrollOffset = selectedChapterIndex - VISIBLE_CHAPTERS + 1;
                refreshChapterRows();
            } else if (kp->code == sf::Keyboard::Key::Enter && !chapters.empty()) {
                activeChapterIndex = selectedChapterIndex;
                const auto& ch = chapters[activeChapterIndex];
                screen = Screen::READING_INTRO;
                titleText->setFillColor(sf::Color(255, 220, 80));
                titleText->setString("Ch " + std::to_string(ch.chapterNumber) + ": " + ch.chapterName);
                contentText->setPosition(sf::Vector2f(90.f, 118.f));
                contentText->setString(wrapText(ch.storyIntro, font, 20, 1100.f));
                navText->setString("ENTER: Continue   ESC: Back to Chapters");
            } else if (kp->code == sf::Keyboard::Key::Escape) {
                screen = Screen::SUBJECT_SELECT;
                titleText->setString("STORY MODE - Select Subject");
                navText->setString("UP/DOWN: Navigate   ENTER: Select   ESC: Menu");
                updateSubjectColors();
            }
            break;
        }

        // ── Story intro ───────────────────────────────────────────────────────
        case Screen::READING_INTRO: {
            const auto& ch = chapters[activeChapterIndex];
            if (kp->code == sf::Keyboard::Key::Enter) {
                screen = Screen::READING_CONCEPT;
                contentText->setString(wrapText(ch.conceptExplanation, font, 20, 1100.f));
            } else if (kp->code == sf::Keyboard::Key::Escape) {
                screen = Screen::CHAPTER_SELECT;
                titleText->setString(game->selectedSubject + " - Chapters");
                navText->setString("UP/DOWN: Navigate   ENTER: Read   ESC: Back");
                refreshChapterRows();
            }
            break;
        }

        // ── Concept explanation ───────────────────────────────────────────────
        case Screen::READING_CONCEPT: {
            const auto& ch = chapters[activeChapterIndex];
            if (kp->code == sf::Keyboard::Key::Enter) {
                screen = Screen::READING_EXAMPLE;
                contentText->setString(wrapText(ch.realLifeExample, font, 20, 1100.f));
                navText->setString("ENTER: Quick Check   ESC: Back to Chapters");
            } else if (kp->code == sf::Keyboard::Key::Escape) {
                screen = Screen::CHAPTER_SELECT;
                titleText->setString(game->selectedSubject + " - Chapters");
                navText->setString("UP/DOWN: Navigate   ENTER: Read   ESC: Back");
                refreshChapterRows();
            }
            break;
        }

        // ── Real-life example ─────────────────────────────────────────────────
        case Screen::READING_EXAMPLE: {
            const auto& ch = chapters[activeChapterIndex];
            if (kp->code == sf::Keyboard::Key::Enter) {
                screen = Screen::QUICK_CHECK;
                selectedAnswerIndex = 0;
                answerSubmitted     = false;
                answerCorrect       = false;
                titleText->setString("Quick Check  -  Ch " + std::to_string(ch.chapterNumber));
                contentText->setPosition(sf::Vector2f(90.f, 118.f));
                contentText->setString(wrapText(ch.quickCheck.question, font, 21, 1100.f));
                for (int i = 0; i < 4; i++) {
                    std::string opt = std::string(1, char('A' + i)) + ".  ";
                    opt += (i < (int)ch.quickCheck.options.size())
                           ? ch.quickCheck.options[i] : "?";
                    answerTexts[i]->setString(wrapText(opt, font, 19, 500.f));
                }
                navText->setString("1/2/3/4 or UP/DOWN + ENTER: Answer   ESC: Back");
                updateAnswerColors();
            } else if (kp->code == sf::Keyboard::Key::Escape) {
                screen = Screen::CHAPTER_SELECT;
                titleText->setString(game->selectedSubject + " - Chapters");
                navText->setString("UP/DOWN: Navigate   ENTER: Read   ESC: Back");
                refreshChapterRows();
            }
            break;
        }

        // ── Quick check ───────────────────────────────────────────────────────
        case Screen::QUICK_CHECK: {
            if (answerSubmitted) break;

            auto submitAnswer = [&](int idx) {
                const auto& ch = chapters[activeChapterIndex];
                selectedAnswerIndex = idx;
                answerSubmitted     = true;
                answerCorrect       = (idx == ch.quickCheck.correct);
                screen = Screen::QUICK_RESULT;

                if (answerCorrect) {
                    titleText->setFillColor(sf::Color(60, 220, 100));
                    titleText->setString("CORRECT!  Well done, Luminary!");
                    game->playerData.correctAnswers++;
                    saveChapterCompletion(ch.chapterNumber);
                } else {
                    titleText->setFillColor(sf::Color(220, 80, 80));
                    titleText->setString("Not quite!  Keep learning!");
                }
                game->playerData.totalQuestionsAnswered++;
                contentText->setPosition(sf::Vector2f(90.f, 462.f));
                contentText->setString(wrapText(
                    "Explanation:  " + ch.quickCheck.explanation, font, 19, 1100.f));
                navText->setString("ENTER or ESC: Back to Chapters");
                updateAnswerColors();
            };

            if      (kp->code == sf::Keyboard::Key::Num1)  { submitAnswer(0); }
            else if (kp->code == sf::Keyboard::Key::Num2)  { submitAnswer(1); }
            else if (kp->code == sf::Keyboard::Key::Num3)  { submitAnswer(2); }
            else if (kp->code == sf::Keyboard::Key::Num4)  { submitAnswer(3); }
            else if (kp->code == sf::Keyboard::Key::Up) {
                selectedAnswerIndex = (selectedAnswerIndex - 1 + 4) % 4;
                updateAnswerColors();
            } else if (kp->code == sf::Keyboard::Key::Down) {
                selectedAnswerIndex = (selectedAnswerIndex + 1) % 4;
                updateAnswerColors();
            } else if (kp->code == sf::Keyboard::Key::Enter) {
                submitAnswer(selectedAnswerIndex);
            } else if (kp->code == sf::Keyboard::Key::Escape) {
                screen = Screen::CHAPTER_SELECT;
                titleText->setFillColor(sf::Color(255, 220, 80));
                titleText->setString(game->selectedSubject + " - Chapters");
                navText->setString("UP/DOWN: Navigate   ENTER: Read   ESC: Back");
                refreshChapterRows();
            }
            break;
        }

        // ── Quick check result ────────────────────────────────────────────────
        case Screen::QUICK_RESULT:
            if (kp->code == sf::Keyboard::Key::Enter || kp->code == sf::Keyboard::Key::Escape) {
                screen = Screen::CHAPTER_SELECT;
                titleText->setFillColor(sf::Color(255, 220, 80));
                titleText->setString(game->selectedSubject + " - Chapters");
                contentText->setPosition(sf::Vector2f(90.f, 118.f));
                navText->setString("UP/DOWN: Navigate   ENTER: Read   ESC: Back");
                refreshChapterRows();
            }
            break;
        }
    }

    // ── Mouse clicks ──────────────────────────────────────────────────────────
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button != sf::Mouse::Button::Left) return;
        sf::Vector2f mp(static_cast<float>(mb->position.x), static_cast<float>(mb->position.y));
        sf::Font& font = game->getMainFont();

        switch (screen) {

        case Screen::SUBJECT_SELECT:
            for (int i = 0; i < (int)subjectBoxes.size(); i++) {
                if (subjectBoxes[i]->getGlobalBounds().contains(mp)) {
                    selectedSubjectIndex = i;
                    game->selectedSubject = subjects[i];
                    loadSubjectChapters(game->selectedSubject);
                    screen = Screen::CHAPTER_SELECT;
                    titleText->setString(game->selectedSubject + " - Chapters");
                    navText->setString("UP/DOWN: Navigate   ENTER: Read   ESC: Back");
                    break;
                }
            }
            break;

        case Screen::CHAPTER_SELECT:
            for (int row = 0; row < (int)chapterBoxes.size(); row++) {
                if (chapterBoxes[row]->getGlobalBounds().contains(mp)) {
                    int chIdx = chapterScrollOffset + row;
                    if (chIdx < (int)chapters.size()) {
                        selectedChapterIndex = chIdx;
                        activeChapterIndex   = chIdx;
                        const auto& ch = chapters[chIdx];
                        screen = Screen::READING_INTRO;
                        titleText->setFillColor(sf::Color(255, 220, 80));
                        titleText->setString("Ch " + std::to_string(ch.chapterNumber) + ": " + ch.chapterName);
                        contentText->setPosition(sf::Vector2f(90.f, 118.f));
                        contentText->setString(wrapText(ch.storyIntro, font, 20, 1100.f));
                        navText->setString("ENTER: Continue   ESC: Back to Chapters");
                    }
                    break;
                }
            }
            break;

        case Screen::READING_INTRO:
        case Screen::READING_CONCEPT:
        case Screen::READING_EXAMPLE: {
            // Click anywhere to advance (same as Enter)
            const auto& ch = chapters[activeChapterIndex];
            if (screen == Screen::READING_INTRO) {
                screen = Screen::READING_CONCEPT;
                contentText->setString(wrapText(ch.conceptExplanation, font, 20, 1100.f));
            } else if (screen == Screen::READING_CONCEPT) {
                screen = Screen::READING_EXAMPLE;
                contentText->setString(wrapText(ch.realLifeExample, font, 20, 1100.f));
                navText->setString("ENTER: Quick Check   ESC: Back to Chapters");
            } else {
                screen = Screen::QUICK_CHECK;
                selectedAnswerIndex = 0;
                answerSubmitted     = false;
                answerCorrect       = false;
                titleText->setString("Quick Check  -  Ch " + std::to_string(ch.chapterNumber));
                contentText->setPosition(sf::Vector2f(90.f, 118.f));
                contentText->setString(wrapText(ch.quickCheck.question, font, 21, 1100.f));
                for (int i = 0; i < 4; i++) {
                    std::string opt = std::string(1, char('A' + i)) + ".  ";
                    opt += (i < (int)ch.quickCheck.options.size()) ? ch.quickCheck.options[i] : "?";
                    answerTexts[i]->setString(wrapText(opt, font, 19, 500.f));
                }
                navText->setString("1/2/3/4 or Click: Answer   ESC: Back");
                updateAnswerColors();
            }
            break;
        }

        case Screen::QUICK_CHECK:
            if (!answerSubmitted) {
                for (int i = 0; i < (int)answerBoxes.size(); i++) {
                    if (answerBoxes[i]->getGlobalBounds().contains(mp)) {
                        const auto& ch = chapters[activeChapterIndex];
                        selectedAnswerIndex = i;
                        answerSubmitted     = true;
                        answerCorrect       = (i == ch.quickCheck.correct);
                        screen = Screen::QUICK_RESULT;
                        if (answerCorrect) {
                            titleText->setFillColor(sf::Color(60, 220, 100));
                            titleText->setString("CORRECT!  Well done, Luminary!");
                            game->playerData.correctAnswers++;
                            saveChapterCompletion(ch.chapterNumber);
                        } else {
                            titleText->setFillColor(sf::Color(220, 80, 80));
                            titleText->setString("Not quite!  Keep learning!");
                        }
                        game->playerData.totalQuestionsAnswered++;
                        contentText->setPosition(sf::Vector2f(90.f, 462.f));
                        contentText->setString(wrapText(
                            "Explanation:  " + ch.quickCheck.explanation, font, 19, 1100.f));
                        navText->setString("ENTER or ESC: Back to Chapters");
                        updateAnswerColors();
                        break;
                    }
                }
            }
            break;

        case Screen::QUICK_RESULT:
            screen = Screen::CHAPTER_SELECT;
            titleText->setFillColor(sf::Color(255, 220, 80));
            titleText->setString(game->selectedSubject + " - Chapters");
            contentText->setPosition(sf::Vector2f(90.f, 118.f));
            navText->setString("UP/DOWN: Navigate   ENTER: Read   ESC: Back");
            refreshChapterRows();
            break;
        }
    }
}

// ─── Update ──────────────────────────────────────────────────────────────────

void TrainingStoryState::update(float /*deltaTime*/) {}

// ─── Render ──────────────────────────────────────────────────────────────────

void TrainingStoryState::render(sf::RenderWindow& window) {
    sf::Font& font = game->getMainFont();

    // Centre the title every frame (string may change)
    {
        sf::FloatRect tb = titleText->getLocalBounds();
        titleText->setPosition(sf::Vector2f(640.f - tb.size.x * 0.5f, 28.f));
    }
    drawStoryText(window, *titleText, titleText->getPosition());

    // Bottom nav strip
    drawStoryPanel(window, bottomPanel->getGlobalBounds(), sf::Color(10, 8, 28, 190));
    drawStoryText(window, *navText, navText->getPosition());

    switch (screen) {

    // ── Subject selection ─────────────────────────────────────────────────────
    case Screen::SUBJECT_SELECT: {
        sf::Text sub(font, "Choose a subject to explore:", 21);
        sub.setFillColor(sf::Color(155, 155, 210));
        sub.setPosition(sf::Vector2f(640.f - sub.getLocalBounds().size.x * 0.5f, 118.f));
        window.draw(sub);
        for (int i = 0; i < 5; i++) {
            drawStoryPanel(window, subjectBoxes[i]->getGlobalBounds(), sf::Color(28, 14, 52, 210));
            drawStoryText(window, *subjectTexts[i], subjectTexts[i]->getPosition());
        }
        break;
    }

    // ── Chapter selection ─────────────────────────────────────────────────────
    case Screen::CHAPTER_SELECT: {
        int total = static_cast<int>(chapters.size());
        for (int i = 0; i < VISIBLE_CHAPTERS; i++) {
            int idx = chapterScrollOffset + i;
            if (idx < total) {
                drawStoryPanel(window, chapterBoxes[i]->getGlobalBounds(), sf::Color(28, 14, 52, 185));
                drawStoryText(window, *chapterTexts[i], chapterTexts[i]->getPosition());
            }
        }
        if (chapterScrollOffset > 0) {
            sf::Text up(font, "^  more above", 15);
            up.setFillColor(sf::Color(140, 140, 200));
            up.setPosition(sf::Vector2f(600.f, 96.f));
            window.draw(up);
        }
        if (chapterScrollOffset + VISIBLE_CHAPTERS < total) {
            sf::Text dn(font, "v  more below", 15);
            dn.setFillColor(sf::Color(140, 140, 200));
            dn.setPosition(sf::Vector2f(600.f, 579.f));
            window.draw(dn);
        }
        break;
    }

    // ── Reading screens (intro / concept / example) ───────────────────────────
    case Screen::READING_INTRO:
    case Screen::READING_CONCEPT:
    case Screen::READING_EXAMPLE: {
        const char* partLabels[] = { "Story", "Concept", "Real-Life Example" };
        int pi = (screen == Screen::READING_INTRO)   ? 0
               : (screen == Screen::READING_CONCEPT) ? 1 : 2;
        sf::Text tag(font, std::string("[ ") + partLabels[pi] + " ]", 18);
        tag.setFillColor(sf::Color(130, 150, 255, 200));
        tag.setPosition(sf::Vector2f(90.f, 72.f));
        window.draw(tag);
        drawStoryPanel(window, contentPanel->getGlobalBounds(), sf::Color(14, 10, 38, 148));
        drawStoryText(window, *contentText, contentText->getPosition());
        break;
    }

    // ── Quick check ───────────────────────────────────────────────────────────
    case Screen::QUICK_CHECK: {
        // Question panel
        drawStoryPanel(window,
                       sf::FloatRect(sf::Vector2f(60.f, 92.f), sf::Vector2f(1160.f, 180.f)),
                       sf::Color(12, 8, 32, 160));
        drawStoryText(window, *contentText, contentText->getPosition());
        // Answer boxes
        for (int i = 0; i < 4; i++) {
            drawStoryPanel(window, answerBoxes[i]->getGlobalBounds(), sf::Color(28, 14, 52, 185));
            drawStoryText(window, *answerTexts[i], answerTexts[i]->getPosition());
        }
        break;
    }

    // ── Quick check result ────────────────────────────────────────────────────
    case Screen::QUICK_RESULT: {
        // Show answer boxes (already coloured by updateAnswerColors)
        for (int i = 0; i < 4; i++) {
            drawStoryPanel(window, answerBoxes[i]->getGlobalBounds(), sf::Color(28, 14, 52, 185));
            drawStoryText(window, *answerTexts[i], answerTexts[i]->getPosition());
        }
        // Explanation panel
        drawStoryPanel(window,
                       sf::FloatRect(sf::Vector2f(60.f, 448.f), sf::Vector2f(1160.f, 205.f)),
                       sf::Color(14, 10, 38, 150));
        drawStoryText(window, *contentText, contentText->getPosition());
        break;
    }
    }
}

// ─── Lifecycle ───────────────────────────────────────────────────────────────

void TrainingStoryState::onEnter() {
    std::cout << "Entered Story Mode\n";
    storyManager.loadAllStories();
    screen               = Screen::SUBJECT_SELECT;
    selectedSubjectIndex = 0;
    titleText->setFillColor(sf::Color(255, 220, 80));
    titleText->setString("STORY MODE - Select Subject");
    contentText->setPosition(sf::Vector2f(90.f, 118.f));
    navText->setString("UP/DOWN: Navigate   ENTER: Select   ESC: Menu");
    updateSubjectColors();
}

void TrainingStoryState::onExit() {
    std::cout << "Exited Story Mode\n";
}
