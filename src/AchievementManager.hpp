#pragma once
#include <string>
#include <vector>
#include <functional>

struct Achievement {
    std::string id;
    std::string name;
    std::string desc;
    bool unlocked = false;
};

class AchievementManager {
public:
    using UnlockCallback = std::function<void(const Achievement&)>;

    AchievementManager();

    void setUsername(const std::string& username);
    void setUnlockCallback(UnlockCallback cb) { onUnlock = cb; }

    // Call these from game logic — silently ignored if already unlocked
    void tryUnlock(const std::string& id);

    // Check compound conditions
    void checkStreak(int streak);
    void checkQuestionsAnswered(int total);
    void checkCharacterLevel(int level);
    void checkFirstCorrect();
    void checkFirstWin();
    void checkStoryChapter();
    void checkAllSubjects(const std::vector<std::string>& subjectsAnswered);

    const std::vector<Achievement>& getAll() const { return achievements; }
    int getUnlockedCount() const;

    void save();
    void load();

private:
    std::vector<Achievement> achievements;
    std::string saveFilePath;
    UnlockCallback onUnlock;

    void loadDefinitions();
};
