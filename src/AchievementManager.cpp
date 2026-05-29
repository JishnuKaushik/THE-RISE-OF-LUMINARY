#include "AchievementManager.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>

// ── Tiny JSON helpers (same pattern as StoryManager) ─────────────────────────
static std::string acStr(const std::string& obj, const std::string& key) {
    std::string sk = "\"" + key + "\"";
    size_t kp = obj.find(sk);
    if (kp == std::string::npos) return "";
    size_t cp = obj.find(':', kp + sk.size());
    if (cp == std::string::npos) return "";
    size_t q = obj.find('"', cp + 1);
    if (q == std::string::npos) return "";
    std::string out;
    for (size_t i = q + 1; i < obj.size() && obj[i] != '"'; i++) out += obj[i];
    return out;
}

AchievementManager::AchievementManager() {
    loadDefinitions();
}

void AchievementManager::loadDefinitions() {
    achievements.clear();
    std::ifstream file("data/achievements.json");
    if (!file.is_open()) {
        // Fallback hardcoded definitions
        achievements = {
            {"first_correct", "First Blood",      "Answer your first question correctly.", false},
            {"first_win",     "First Victory",    "Complete a chapter for the first time.", false},
            {"streak_5",      "On a Roll",        "Reach a streak of 5.", false},
            {"streak_10",     "On Fire!",         "Reach a streak of 10.", false},
            {"questions_50",  "Half Century",     "Answer 50 questions.", false},
            {"questions_100", "Century Scholar",  "Answer 100 questions.", false},
            {"level_5",       "Rising Star",      "Reach character level 5.", false},
            {"level_10",      "Champion",         "Reach character level 10.", false},
            {"story_chapter", "Story Begins",     "Complete a story chapter.", false},
            {"all_subjects",  "Renaissance Mind", "Answer in all 5 subjects.", false}
        };
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    // Parse JSON array of achievement objects
    size_t pos = 0;
    while (pos < content.size()) {
        size_t os = content.find('{', pos);
        if (os == std::string::npos) break;
        size_t oe = content.find('}', os);
        if (oe == std::string::npos) break;
        std::string obj = content.substr(os, oe - os + 1);
        Achievement a;
        a.id      = acStr(obj, "id");
        a.name    = acStr(obj, "name");
        a.desc    = acStr(obj, "desc");
        a.unlocked = false;
        if (!a.id.empty()) achievements.push_back(a);
        pos = oe + 1;
    }
}

void AchievementManager::setUsername(const std::string& username) {
    saveFilePath = "saves/" + username + "_achievements.json";
    load();
}

void AchievementManager::tryUnlock(const std::string& id) {
    for (auto& a : achievements) {
        if (a.id == id && !a.unlocked) {
            a.unlocked = true;
            std::cout << "[Achievement] Unlocked: " << a.name << "\n";
            if (onUnlock) onUnlock(a);
            save();
            return;
        }
    }
}

void AchievementManager::checkStreak(int streak) {
    if (streak >= 5)  tryUnlock("streak_5");
    if (streak >= 10) tryUnlock("streak_10");
}

void AchievementManager::checkQuestionsAnswered(int total) {
    if (total >= 50)  tryUnlock("questions_50");
    if (total >= 100) tryUnlock("questions_100");
}

void AchievementManager::checkCharacterLevel(int level) {
    if (level >= 5)  tryUnlock("level_5");
    if (level >= 10) tryUnlock("level_10");
}

void AchievementManager::checkFirstCorrect() { tryUnlock("first_correct"); }
void AchievementManager::checkFirstWin()     { tryUnlock("first_win"); }
void AchievementManager::checkStoryChapter() { tryUnlock("story_chapter"); }

void AchievementManager::checkAllSubjects(const std::vector<std::string>& subjectsAnswered) {
    static const std::vector<std::string> ALL = {
        "Mathematics","Science","History","Literature","Arts"
    };
    for (const auto& s : ALL) {
        bool found = false;
        for (const auto& sa : subjectsAnswered) if (sa == s) { found = true; break; }
        if (!found) return;
    }
    tryUnlock("all_subjects");
}

int AchievementManager::getUnlockedCount() const {
    int n = 0;
    for (const auto& a : achievements) if (a.unlocked) n++;
    return n;
}

void AchievementManager::save() {
    if (saveFilePath.empty()) return;
    std::filesystem::create_directory("saves");
    std::ofstream file(saveFilePath);
    if (!file.is_open()) return;
    file << "[\n";
    for (size_t i = 0; i < achievements.size(); i++) {
        const auto& a = achievements[i];
        file << "  {\"id\": \"" << a.id << "\", \"unlocked\": "
             << (a.unlocked ? "true" : "false") << "}";
        if (i < achievements.size() - 1) file << ",";
        file << "\n";
    }
    file << "]\n";
}

void AchievementManager::load() {
    if (saveFilePath.empty()) return;
    std::ifstream file(saveFilePath);
    if (!file.is_open()) return;
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    size_t pos = 0;
    while (pos < content.size()) {
        size_t os = content.find('{', pos);
        if (os == std::string::npos) break;
        size_t oe = content.find('}', os);
        if (oe == std::string::npos) break;
        std::string obj = content.substr(os, oe - os + 1);
        std::string id  = acStr(obj, "id");
        bool unlocked   = (obj.find("true") != std::string::npos);
        for (auto& a : achievements)
            if (a.id == id) { a.unlocked = unlocked; break; }
        pos = oe + 1;
    }
}
