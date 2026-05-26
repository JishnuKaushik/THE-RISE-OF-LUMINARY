#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Character.hpp"
#include <optional>

struct Enemy {
    std::string name;
    Element element;
    int health;
    int maxHealth;
    int attack;
    int defense;
    int level;
    int luminescenceReward;
    int xpReward;
    std::string spriteName;
};

class BattleSystem {
private:
    Character* player;
    sf::Texture backgroundTexture;
    std::optional<sf::Sprite> backgroundSprite;
    Enemy currentEnemy;
    bool playerTurn;
    bool battleActive;
    std::string battleLog;
    float logTimer;
    
    // Damage calculation with element system
    float calculateElementMultiplier(Element attacker, Element defender);
    int calculateDamage(int baseDamage, int enemyDefense);
    
public:
    BattleSystem();
    ~BattleSystem();
    
    void startBattle(Character* playerCharacter);
    void playerAttack(int damage, Element attackElement);
    void enemyAttack();
    void healPlayer(int amount);
    void endBattle();
    
    bool isBattleActive() const { return battleActive; }
    bool isPlayerTurn() const { return playerTurn; }
    Enemy getCurrentEnemy() const { return currentEnemy; }
    std::string getBattleLog() const { return battleLog; }
    void updateLog(const std::string& message);
    void update(float deltaTime);
    
    // Generate random enemy based on difficulty
    void generateRandomEnemy(int playerLevel);
};
