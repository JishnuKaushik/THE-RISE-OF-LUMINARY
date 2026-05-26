#pragma once
#include <string>
#include <vector>
#include <map>

struct StoryChapter {
    int chapterNumber;
    std::string chapterName;
    std::string storyIntro;
    std::string conceptExplanation;
    std::string realLifeExample;
    struct {
        std::string question;
        std::vector<std::string> options;
        int correct;
        std::string explanation;
    } quickCheck;
};

class StoryManager {
private:
    std::map<std::string, std::vector<StoryChapter>> storiesBySubject;
    void loadStoriesFromFile(const std::string& filepath);
    
public:
    StoryManager();
    ~StoryManager();
    void loadAllStories();
    std::vector<StoryChapter> getStoriesBySubject(const std::string& subject);
    int getTotalChapters(const std::string& subject);
    void printStats();
};
