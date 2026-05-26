#pragma once
#include <string>
#include <vector>
#include "Character.hpp"

enum class EnemyRarity {
    COMMON,
    RARE,
    EPIC,
    LEGENDARY
};

struct Enemy {
    int id;
    std::string name;
    Element element;
    EnemyRarity rarity;
    int health;
    int maxHealth;
    int attack;
    int defense;
    int level;
    int expReward;
    int luminescenceReward;
    std::string spriteName;
    
    Enemy() : id(0), element(Element::FIRE), rarity(EnemyRarity::COMMON),
              health(50), maxHealth(50), attack(15), defense(5),
              level(1), expReward(25), luminescenceReward(50) {}
    
    std::string getRarityString() const;
    std::string getElementString() const;
    void takeDamage(int amount);
    bool isAlive() const { return health > 0; }
};

class EnemyManager {
private:
    std::vector<Enemy> allEnemies;
    
    void loadDefaultEnemies();
    
public:
    EnemyManager();
    ~EnemyManager();
    
    void initialize();
    Enemy getRandomEnemy(int playerLevel);
    Enemy getEnemyById(int id);
    std::vector<Enemy> getEnemiesByRarity(EnemyRarity rarity);
};
