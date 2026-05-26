#pragma once
#include <string>
#include <vector>

enum class Element {
    FIRE,
    WATER,
    LIGHTNING,
    NATURE,
    DARK,
    LIGHT
};

enum class Rarity {
    COMMON,
    RARE,
    EPIC,
    LEGENDARY
};

struct Character {
    int id;
    std::string name;
    Element element;
    Rarity rarity;
    int health;
    int maxHealth;
    int attack;
    int defense;
    int level;
    int currentXP;
    int unlockCost;
    bool isUnlocked;
    std::string spriteName;
    std::string specialAbility;
    std::string abilityDescription;
    
    Character() : id(0), element(Element::FIRE), rarity(Rarity::COMMON),
                  health(100), maxHealth(100), attack(20), defense(10),
                  level(1), currentXP(0), unlockCost(0), isUnlocked(false) {}
    
    std::string getElementString() const;
    std::string getRarityString() const;
    int getRequiredXPForNextLevel() const;
    
    void addXP(int amount);
    void levelUp();
    void heal(int amount);
    void takeDamage(int amount);
    bool isAlive() const { return health > 0; }
};

class CharacterManager {
private:
    std::vector<Character> allCharacters;
    Character* selectedCharacter;
    std::string saveFilePath;
    
    void loadCharactersFromFile();
    void saveCharactersToFile();
    
public:
    CharacterManager();
    ~CharacterManager();
    
    void initialize();
    std::vector<Character> getUnlockedCharacters() const;
    std::vector<Character> getLockedCharacters() const;
    bool unlockCharacter(int characterId);
    bool selectCharacter(int characterId);
    Character* getSelectedCharacter() { return selectedCharacter; }
    Character* getCharacterById(int id);
    void saveProgress();
    void loadProgress();
    int getTotalLuminescence() const;
    void addLuminescence(int amount);
    bool spendLuminescence(int amount);
};
