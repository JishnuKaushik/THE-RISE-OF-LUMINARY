#pragma once
#include <string>
#include <vector>
#include <map>

struct PlayerData {
    std::string username;
    int age;
    std::string gradeLevel;
    std::string selectedSubject;
    int totalScore;
    int bestStreak;
    int totalQuestionsAnswered;
    int correctAnswers;
    std::vector<int> completedChapters;
    std::string lastPlayedDate;
    int currentStreak;
    int dailyMinutes;
    int selectedCharacter;
    std::map<std::string, int> topicAttempts;  // subject → total attempts
    std::map<std::string, int> topicCorrect;   // subject → correct answers
    // Daily quest tracking (reset each new calendar day)
    int dailyQuestionsAnswered;
    int dailyChaptersCompleted;
    bool dailyQuestStreakDone;   // logged in 2+ consecutive days
};

class SaveManager {
public:
    static bool savePlayer(const PlayerData& data);
    static bool loadPlayer(const std::string& username, PlayerData& data);
    static std::vector<std::string> getSavedProfiles();
    static bool deleteProfile(const std::string& username);
    static bool profileExists(const std::string& username);
    
private:
    static std::string getSavePath(const std::string& username);
};
