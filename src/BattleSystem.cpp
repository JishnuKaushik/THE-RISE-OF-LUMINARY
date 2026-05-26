#include "BattleSystem.hpp"
#include <iostream>
#include <random>
#include <cmath>

BattleSystem::BattleSystem() : player(nullptr), playerTurn(true), battleActive(false), logTimer(0) {

    if (backgroundTexture.loadFromFile("assets/images/battlefield.png")) {
        backgroundSprite.emplace(backgroundTexture);

        backgroundSprite->setScale(
            sf::Vector2f(
                1280.0f / backgroundTexture.getSize().x,
                720.0f / backgroundTexture.getSize().y
            )
        );
    }
}

BattleSystem::~BattleSystem() {
}

float BattleSystem::calculateElementMultiplier(Element attacker, Element defender) {
    // Element weakness chart
    // Strong: 2.0x damage, Weak: 0.5x damage, Normal: 1.0x
    
    switch (attacker) {
        case Element::FIRE:
            if (defender == Element::NATURE) return 2.0f;
            if (defender == Element::WATER) return 0.5f;
            break;
        case Element::WATER:
            if (defender == Element::FIRE) return 2.0f;
            if (defender == Element::LIGHTNING) return 0.5f;
            break;
        case Element::LIGHTNING:
            if (defender == Element::WATER) return 2.0f;
            if (defender == Element::NATURE) return 0.5f;
            break;
        case Element::NATURE:
            if (defender == Element::WATER) return 2.0f;
            if (defender == Element::FIRE) return 0.5f;
            break;
        case Element::LIGHT:
            if (defender == Element::DARK) return 2.0f;
            break;
        case Element::DARK:
            if (defender == Element::LIGHT) return 2.0f;
            break;
        default:
            break;
    }
    return 1.0f;
}

int BattleSystem::calculateDamage(int baseDamage, int enemyDefense) {
    int damage = baseDamage - (enemyDefense / 2);
    if (damage < 5) damage = 5;
    return damage;
}

void BattleSystem::generateRandomEnemy(int playerLevel) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    int roll = dis(gen);
    
    std::vector<Enemy> enemies;
    
    // Common enemies (50% chance)
    Enemy goblin;
    goblin.name = "🐺 GOBLIN";
    goblin.element = Element::DARK;
    goblin.health = 50 + playerLevel * 10;
    goblin.maxHealth = goblin.health;
    goblin.attack = 15 + playerLevel * 2;
    goblin.defense = 5 + playerLevel;
    goblin.level = playerLevel;
    goblin.luminescenceReward = 50;
    goblin.xpReward = 25;
    enemies.push_back(goblin);
    
    Enemy slime;
    slime.name = "💚 SLIME";
    slime.element = Element::NATURE;
    slime.health = 40 + playerLevel * 8;
    slime.maxHealth = slime.health;
    slime.attack = 10 + playerLevel * 2;
    slime.defense = 8 + playerLevel;
    slime.level = playerLevel;
    slime.luminescenceReward = 40;
    slime.xpReward = 20;
    enemies.push_back(slime);
    
    // Rare enemies (30% chance)
    Enemy orc;
    orc.name = "👹 ORC";
    orc.element = Element::FIRE;
    orc.health = 80 + playerLevel * 15;
    orc.maxHealth = orc.health;
    orc.attack = 25 + playerLevel * 3;
    orc.defense = 10 + playerLevel;
    orc.level = playerLevel;
    orc.luminescenceReward = 100;
    orc.xpReward = 50;
    enemies.push_back(orc);
    
    Enemy wolf;
    wolf.name = "🐺 ICE WOLF";
    wolf.element = Element::WATER;
    wolf.health = 70 + playerLevel * 12;
    wolf.maxHealth = wolf.health;
    wolf.attack = 20 + playerLevel * 3;
    wolf.defense = 12 + playerLevel;
    wolf.level = playerLevel;
    wolf.luminescenceReward = 90;
    wolf.xpReward = 45;
    enemies.push_back(wolf);
    
    // Epic enemies (15% chance)
    Enemy dragon;
    dragon.name = "🐉 FIRE DRAGON";
    dragon.element = Element::FIRE;
    dragon.health = 150 + playerLevel * 20;
    dragon.maxHealth = dragon.health;
    dragon.attack = 40 + playerLevel * 4;
    dragon.defense = 20 + playerLevel * 2;
    dragon.level = playerLevel;
    dragon.luminescenceReward = 200;
    dragon.xpReward = 100;
    enemies.push_back(dragon);
    
    // Legendary enemies (5% chance)
    Enemy ancientLich;
    ancientLich.name = "💀 ANCIENT LICH";
    ancientLich.element = Element::DARK;
    ancientLich.health = 250 + playerLevel * 30;
    ancientLich.maxHealth = ancientLich.health;
    ancientLich.attack = 60 + playerLevel * 5;
    ancientLich.defense = 30 + playerLevel * 2;
    ancientLich.level = playerLevel;
    ancientLich.luminescenceReward = 500;
    ancientLich.xpReward = 250;
    enemies.push_back(ancientLich);
    
    // Select enemy based on roll
    if (roll <= 50) {
        // Common enemy
        std::uniform_int_distribution<> commonDis(0, 1);
        currentEnemy = enemies[commonDis(gen)];
    } else if (roll <= 80) {
        // Rare enemy
        std::uniform_int_distribution<> rareDis(2, 3);
        currentEnemy = enemies[rareDis(gen)];
    } else if (roll <= 95) {
        // Epic enemy
        currentEnemy = enemies[4];
    } else {
        // Legendary enemy
        currentEnemy = enemies[5];
    }
    
    updateLog("⚔️ " + currentEnemy.name + " appears!");
    updateLog("💀 " + currentEnemy.name + " has " + std::to_string(currentEnemy.health) + " HP");
}

void BattleSystem::startBattle(Character* playerCharacter) {
    player = playerCharacter;
    generateRandomEnemy(player->level);
    playerTurn = true;
    battleActive = true;
    updateLog("⚔️ BATTLE STARTED!");
    updateLog("✨ Your turn! Answer correctly to attack!");
}

void BattleSystem::playerAttack(int damage, Element attackElement) {
    if (!battleActive || !playerTurn) return;
    
    float multiplier = calculateElementMultiplier(attackElement, currentEnemy.element);
    int finalDamage = calculateDamage(damage, currentEnemy.defense);
    finalDamage = static_cast<int>(finalDamage * multiplier);
    
    std::string elementText;
    switch (attackElement) {
        case Element::FIRE: elementText = "🔥 FIRE"; break;
        case Element::WATER: elementText = "💧 WATER"; break;
        case Element::LIGHTNING: elementText = "⚡ LIGHTNING"; break;
        default: elementText = "✨ MAGIC";
    }
    
    updateLog("✨ You cast " + elementText + " for " + std::to_string(finalDamage) + " damage!");
    
    if (multiplier > 1.0) {
        updateLog("⭐ SUPER EFFECTIVE! 2x damage!");
    } else if (multiplier < 1.0) {
        updateLog("⚠️ NOT VERY EFFECTIVE... half damage");
    }
    
    currentEnemy.health -= finalDamage;
    
    if (currentEnemy.health <= 0) {
        currentEnemy.health = 0;
        updateLog("💀 " + currentEnemy.name + " defeated!");
        endBattle();
    } else {
        updateLog(currentEnemy.name + " has " + std::to_string(currentEnemy.health) + " HP remaining");
        playerTurn = false;
    }
}

void BattleSystem::enemyAttack() {
    if (!battleActive || playerTurn) return;
    
    int damage = currentEnemy.attack - (player->defense / 2);
    if (damage < 5) damage = 5;
    
    player->takeDamage(damage);
    updateLog("💢 " + currentEnemy.name + " attacks for " + std::to_string(damage) + " damage!");
    updateLog("❤️ Your HP: " + std::to_string(player->health) + "/" + std::to_string(player->maxHealth));
    
    if (!player->isAlive()) {
        updateLog("💀 YOU HAVE BEEN DEFEATED!");
        battleActive = false;
    } else {
        playerTurn = true;
        updateLog("✨ Your turn! Answer correctly to counterattack!");
    }
}

void BattleSystem::healPlayer(int amount) {
    if (!battleActive) return;
    player->heal(amount);
    updateLog("💚 You healed for " + std::to_string(amount) + " HP!");
    updateLog("❤️ Your HP: " + std::to_string(player->health) + "/" + std::to_string(player->maxHealth));
}

void BattleSystem::endBattle() {
    battleActive = false;
    if (player->isAlive()) {
        player->addXP(currentEnemy.xpReward);
        updateLog("🎉 VICTORY!");
        updateLog("✨ Gained " + std::to_string(currentEnemy.xpReward) + " XP!");
        updateLog("💰 Gained " + std::to_string(currentEnemy.luminescenceReward) + " Luminescence!");
    } else {
        updateLog("😔 GAME OVER... Try again!");
    }
}

void BattleSystem::updateLog(const std::string& message) {
    battleLog = message;
    logTimer = 3.0f;
    std::cout << message << std::endl;
}

void BattleSystem::update(float deltaTime) {
    if (logTimer > 0) {
        logTimer -= deltaTime;
        if (logTimer <= 0) {
            battleLog = "";
        }
    }
}
