#include "SaveManager.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <chrono>
#include <ctime>

std::string SaveManager::getSavePath(const std::string& username) {
    std::string savesDir = "saves";
    
    if (!std::filesystem::exists(savesDir)) {
        std::filesystem::create_directory(savesDir);
    }
    
    return savesDir + "/" + username + ".json";
}

bool SaveManager::savePlayer(const PlayerData& data) {
    std::string filepath = getSavePath(data.username);
    std::ofstream file(filepath);
    
    if (!file.is_open()) {
        std::cerr << "Could not save to " << filepath << std::endl;
        return false;
    }
    
    file << "{\n";
    file << "  \"username\": \"" << data.username << "\",\n";
    file << "  \"age\": " << data.age << ",\n";
    file << "  \"gradeLevel\": \"" << data.gradeLevel << "\",\n";
    file << "  \"selectedSubject\": \"" << data.selectedSubject << "\",\n";
    file << "  \"totalScore\": " << data.totalScore << ",\n";
    file << "  \"bestStreak\": " << data.bestStreak << ",\n";
    file << "  \"totalQuestionsAnswered\": " << data.totalQuestionsAnswered << ",\n";
    file << "  \"correctAnswers\": " << data.correctAnswers << ",\n";
    file << "  \"lastPlayedDate\": \"" << data.lastPlayedDate << "\",\n";
    file << "  \"currentStreak\": " << data.currentStreak << ",\n";
    file << "  \"dailyMinutes\": " << data.dailyMinutes << ",\n";
    file << "  \"selectedCharacter\": " << data.selectedCharacter << "\n";
    file << "}\n";
    
    file.close();
    std::cout << "Saved profile to: " << filepath << std::endl;
    return true;
}

bool SaveManager::loadPlayer(const std::string& username, PlayerData& data) {
    std::string filepath = getSavePath(username);
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        std::cerr << "Could not load " << filepath << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("\"username\"") != std::string::npos) {
            size_t start = line.find('"');
            start = line.find('"', start + 1);
            size_t end = line.find('"', start + 1);
            data.username = line.substr(start + 1, end - start - 1);
        }
        else if (line.find("\"age\"") != std::string::npos) {
            size_t colon = line.find(':');
            data.age = std::stoi(line.substr(colon + 1));
        }
        else if (line.find("\"gradeLevel\"") != std::string::npos) {
            size_t start = line.find('"');
            start = line.find('"', start + 1);
            size_t end = line.find('"', start + 1);
            data.gradeLevel = line.substr(start + 1, end - start - 1);
        }
        else if (line.find("\"selectedSubject\"") != std::string::npos) {
            size_t start = line.find('"');
            start = line.find('"', start + 1);
            size_t end = line.find('"', start + 1);
            data.selectedSubject = line.substr(start + 1, end - start - 1);
        }
        else if (line.find("\"totalScore\"") != std::string::npos) {
            size_t colon = line.find(':');
            data.totalScore = std::stoi(line.substr(colon + 1));
        }
        else if (line.find("\"bestStreak\"") != std::string::npos) {
            size_t colon = line.find(':');
            data.bestStreak = std::stoi(line.substr(colon + 1));
        }
        else if (line.find("\"totalQuestionsAnswered\"") != std::string::npos) {
            size_t colon = line.find(':');
            data.totalQuestionsAnswered = std::stoi(line.substr(colon + 1));
        }
        else if (line.find("\"correctAnswers\"") != std::string::npos) {
            size_t colon = line.find(':');
            data.correctAnswers = std::stoi(line.substr(colon + 1));
        }
        else if (line.find("\"currentStreak\"") != std::string::npos) {
            size_t colon = line.find(':');
            data.currentStreak = std::stoi(line.substr(colon + 1));
        }
        else if (line.find("\"selectedCharacter\"") != std::string::npos) {
            size_t colon = line.find(':');
            data.selectedCharacter = std::stoi(line.substr(colon + 1));
        }
    }
    
    file.close();
    std::cout << "Loaded profile from: " << filepath << std::endl;
    return true;
}

std::vector<std::string> SaveManager::getSavedProfiles() {
    std::vector<std::string> profiles;
    std::string savesDir = "saves";
    
    if (!std::filesystem::exists(savesDir)) {
        return profiles;
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(savesDir)) {
        if (entry.path().extension() == ".json") {
            std::string filename = entry.path().stem().string();
            // Skip character data file
            if (filename != "characters") {
                profiles.push_back(filename);
            }
        }
    }
    
    return profiles;
}

bool SaveManager::deleteProfile(const std::string& username) {
    std::string filepath = getSavePath(username);
    if (std::filesystem::exists(filepath)) {
        std::filesystem::remove(filepath);
        return true;
    }
    return false;
}

bool SaveManager::profileExists(const std::string& username) {
    return std::filesystem::exists(getSavePath(username));
}
