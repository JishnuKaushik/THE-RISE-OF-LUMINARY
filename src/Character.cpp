#include "Character.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>

std::string Character::getElementString() const {
    switch (element) {
        case Element::FIRE: return "🔥 FIRE";
        case Element::WATER: return "💧 WATER";
        case Element::LIGHTNING: return "⚡ LIGHTNING";
        case Element::NATURE: return "🌿 NATURE";
        case Element::DARK: return "🌑 DARK";
        case Element::LIGHT: return "✨ LIGHT";
        default: return "UNKNOWN";
    }
}

std::string Character::getRarityString() const {
    switch (rarity) {
        case Rarity::COMMON: return "COMMON";
        case Rarity::RARE: return "RARE";
        case Rarity::EPIC: return "EPIC";
        case Rarity::LEGENDARY: return "LEGENDARY";
        default: return "COMMON";
    }
}

int Character::getRequiredXPForNextLevel() const {
    return level * 100;
}

void Character::addXP(int amount) {
    currentXP += amount;
    while (currentXP >= getRequiredXPForNextLevel()) {
        currentXP -= getRequiredXPForNextLevel();
        levelUp();
    }
}

void Character::levelUp() {
    level++;
    maxHealth += 10;
    health = maxHealth;
    attack += 5;
    defense += 3;
    std::cout << name << " reached level " << level << "!" << std::endl;

    // Milestone bonuses at levels 5, 10, 15
    if (level == 5) {
        attack += 5;
        std::cout << name << " MILESTONE Lv.5: +5 Attack! (now " << attack << ")\n";
    } else if (level == 10) {
        defense += 3;
        std::cout << name << " MILESTONE Lv.10: +3 Defense! (now " << defense << ")\n";
    } else if (level == 15) {
        maxHealth += 20;
        health = maxHealth;
        std::cout << name << " MILESTONE Lv.15: +20 Max HP! (now " << maxHealth << ")\n";
    }
}

void Character::heal(int amount) {
    health += amount;
    if (health > maxHealth) health = maxHealth;
}

void Character::takeDamage(int amount) {
    int actualDamage = amount - defense;
    if (actualDamage < 0) actualDamage = 1;
    health -= actualDamage;
    if (health < 0) health = 0;
}

CharacterManager::CharacterManager() : selectedCharacter(nullptr) {
    saveFilePath = "saves/characters.json";
    initialize();
}

CharacterManager::~CharacterManager() {
    saveProgress();
}

void CharacterManager::initialize() {
    loadCharactersFromFile();
    
    if (allCharacters.empty()) {
        Character luminary;
        luminary.id = 1;
        luminary.name = "🌟 LUMINARY";
        luminary.element = Element::LIGHT;
        luminary.rarity = Rarity::COMMON;
        luminary.health = 100;
        luminary.maxHealth = 100;
        luminary.attack = 20;
        luminary.defense = 10;
        luminary.level = 1;
        luminary.unlockCost = 0;
        luminary.isUnlocked = true;
        luminary.spriteName = "player_luminary.png";
        luminary.specialAbility = "Light Burst";
        luminary.abilityDescription = "Increases all damage by 10%";
        allCharacters.push_back(luminary);
        
        Character warrior;
        warrior.id = 2;
        warrior.name = "⚔️ WARRIOR";
        warrior.element = Element::FIRE;
        warrior.rarity = Rarity::COMMON;
        warrior.health = 120;
        warrior.maxHealth = 120;
        warrior.attack = 25;
        warrior.defense = 15;
        warrior.level = 1;
        warrior.unlockCost = 500;
        warrior.isUnlocked = false;
        warrior.spriteName = "player_warrior.png";
        warrior.specialAbility = "Power Strike";
        warrior.abilityDescription = "Double damage on next attack";
        allCharacters.push_back(warrior);
        
        Character mage;
        mage.id = 3;
        mage.name = "🔮 MAGE";
        mage.element = Element::WATER;
        mage.rarity = Rarity::COMMON;
        mage.health = 80;
        mage.maxHealth = 80;
        mage.attack = 30;
        mage.defense = 8;
        mage.level = 1;
        mage.unlockCost = 500;
        mage.isUnlocked = false;
        mage.spriteName = "player_mage.png";
        mage.specialAbility = "Mana Shield";
        mage.abilityDescription = "Reduces incoming damage by 20%";
        allCharacters.push_back(mage);
        
        Character archer;
        archer.id = 4;
        archer.name = "🏹 ARCHER";
        archer.element = Element::LIGHTNING;
        archer.rarity = Rarity::COMMON;
        archer.health = 90;
        archer.maxHealth = 90;
        archer.attack = 28;
        archer.defense = 12;
        archer.level = 1;
        archer.unlockCost = 500;
        archer.isUnlocked = false;
        archer.spriteName = "player_archer.png";
        archer.specialAbility = "Piercing Arrow";
        archer.abilityDescription = "Ignores enemy defense";
        allCharacters.push_back(archer);
        
        Character dragonKnight;
        dragonKnight.id = 5;
        dragonKnight.name = "🐉 DRAGON KNIGHT";
        dragonKnight.element = Element::FIRE;
        dragonKnight.rarity = Rarity::EPIC;
        dragonKnight.health = 150;
        dragonKnight.maxHealth = 150;
        dragonKnight.attack = 35;
        dragonKnight.defense = 20;
        dragonKnight.level = 1;
        dragonKnight.unlockCost = 1500;
        dragonKnight.isUnlocked = false;
        dragonKnight.spriteName = "player_dragon_knight.png";
        dragonKnight.specialAbility = "Dragon's Breath";
        dragonKnight.abilityDescription = "Burns enemy for 3 turns";
        allCharacters.push_back(dragonKnight);
        
        Character kingLuminary;
        kingLuminary.id = 6;
        kingLuminary.name = "👑 KING LUMINARY";
        kingLuminary.element = Element::LIGHT;
        kingLuminary.rarity = Rarity::LEGENDARY;
        kingLuminary.health = 200;
        kingLuminary.maxHealth = 200;
        kingLuminary.attack = 40;
        kingLuminary.defense = 25;
        kingLuminary.level = 1;
        kingLuminary.unlockCost = 3000;
        kingLuminary.isUnlocked = false;
        kingLuminary.spriteName = "player_king.png";
        kingLuminary.specialAbility = "Royal Blessing";
        kingLuminary.abilityDescription = "Heals 20% of max health";
        allCharacters.push_back(kingLuminary);
        
        saveProgress();
    }
    
    for (auto& c : allCharacters) {
        if (c.isUnlocked) {
            selectedCharacter = &c;
            break;
        }
    }
}

void CharacterManager::loadCharactersFromFile() {
    if (!std::filesystem::exists(saveFilePath)) {
        return;
    }
    
    std::ifstream file(saveFilePath);
    if (!file.is_open()) return;
    
    std::string line;
    int currentId = 0;
    
    while (std::getline(file, line)) {
        if (line.find("\"id\"") != std::string::npos) {
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                currentId = std::stoi(line.substr(colon + 1));
                for (auto& c : allCharacters) {
                    if (c.id == currentId) {
                        if (line.find("\"isUnlocked\"") != std::string::npos) {
                            size_t colon2 = line.find(':', colon + 1);
                            if (colon2 != std::string::npos) {
                                std::string val = line.substr(colon2 + 1);
                                c.isUnlocked = (val.find("true") != std::string::npos);
                            }
                        }
                        if (line.find("\"level\"") != std::string::npos) {
                            size_t colon2 = line.find(':', colon + 1);
                            if (colon2 != std::string::npos) {
                                c.level = std::stoi(line.substr(colon2 + 1));
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
    file.close();
}

void CharacterManager::saveCharactersToFile() {
    std::filesystem::create_directory("saves");
    
    std::ofstream file(saveFilePath);
    if (!file.is_open()) return;
    
    file << "{\n  \"characters\": [\n";
    for (size_t i = 0; i < allCharacters.size(); i++) {
        Character& c = allCharacters[i];
        file << "    {\n";
        file << "      \"id\": " << c.id << ",\n";
        file << "      \"name\": \"" << c.name << "\",\n";
        file << "      \"isUnlocked\": " << (c.isUnlocked ? "true" : "false") << ",\n";
        file << "      \"level\": " << c.level << ",\n";
        file << "      \"currentXP\": " << c.currentXP << "\n";
        file << "    }";
        if (i < allCharacters.size() - 1) file << ",";
        file << "\n";
    }
    file << "  ]\n}\n";
    file.close();
}

void CharacterManager::saveProgress() {
    saveCharactersToFile();
}

void CharacterManager::loadProgress() {
    loadCharactersFromFile();
}

std::vector<Character> CharacterManager::getUnlockedCharacters() const {
    std::vector<Character> result;
    for (const auto& c : allCharacters) {
        if (c.isUnlocked) {
            result.push_back(c);
        }
    }
    return result;
}

std::vector<Character> CharacterManager::getLockedCharacters() const {
    std::vector<Character> result;
    for (const auto& c : allCharacters) {
        if (!c.isUnlocked) {
            result.push_back(c);
        }
    }
    return result;
}

bool CharacterManager::unlockCharacter(int characterId) {
    for (auto& c : allCharacters) {
        if (c.id == characterId && !c.isUnlocked) {
            if (getTotalLuminescence() >= c.unlockCost) {
                c.isUnlocked = true;
                spendLuminescence(c.unlockCost);
                saveProgress();
                std::cout << "Unlocked: " << c.name << std::endl;
                return true;
            }
        }
    }
    return false;
}

bool CharacterManager::selectCharacter(int characterId) {
    for (auto& c : allCharacters) {
        if (c.id == characterId && c.isUnlocked) {
            selectedCharacter = &c;
            saveProgress();
            return true;
        }
    }
    return false;
}

Character* CharacterManager::getCharacterById(int id) {
    for (auto& c : allCharacters) {
        if (c.id == id) return &c;
    }
    return nullptr;
}

int CharacterManager::getTotalLuminescence() const {
    return 1000;
}

void CharacterManager::addLuminescence(int amount) {
}

bool CharacterManager::spendLuminescence(int amount) {
    return true;
}
