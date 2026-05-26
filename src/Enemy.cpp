#include "Enemy.hpp"
#include <iostream>
#include <random>

std::string Enemy::getRarityString() const {
    switch (rarity) {
        case EnemyRarity::COMMON: return "COMMON";
        case EnemyRarity::RARE: return "RARE";
        case EnemyRarity::EPIC: return "EPIC";
        case EnemyRarity::LEGENDARY: return "LEGENDARY";
        default: return "COMMON";
    }
}

std::string Enemy::getElementString() const {
    switch (element) {
        case Element::FIRE: return "🔥 FIRE";
        case Element::WATER: return "💧 WATER";
        case Element::LIGHTNING: return "⚡ LIGHTNING";
        case Element::NATURE: return "🌿 NATURE";
        case Element::DARK: return "🌑 DARK";
        case Element::LIGHT: return "✨ LIGHT";
        default: return "⚪ NEUTRAL";
    }
}

void Enemy::takeDamage(int amount) {
    health -= amount;
    if (health < 0) health = 0;
}

EnemyManager::EnemyManager() {
    initialize();
}

EnemyManager::~EnemyManager() {
}

void EnemyManager::initialize() {
    loadDefaultEnemies();
}

void EnemyManager::loadDefaultEnemies() {
    // Common Enemies (50% spawn chance)
    Enemy goblin;
    goblin.id = 1;
    goblin.name = "🐺 GOBLIN";
    goblin.element = Element::DARK;
    goblin.rarity = EnemyRarity::COMMON;
    goblin.health = 50;
    goblin.maxHealth = 50;
    goblin.attack = 15;
    goblin.defense = 5;
    goblin.level = 1;
    goblin.expReward = 25;
    goblin.luminescenceReward = 50;
    allEnemies.push_back(goblin);
    
    Enemy slime;
    slime.id = 2;
    slime.name = "💚 SLIME";
    slime.element = Element::NATURE;
    slime.rarity = EnemyRarity::COMMON;
    slime.health = 40;
    slime.maxHealth = 40;
    slime.attack = 10;
    slime.defense = 8;
    slime.level = 1;
    slime.expReward = 20;
    slime.luminescenceReward = 40;
    allEnemies.push_back(slime);
    
    // Rare Enemies (30% spawn chance)
    Enemy orc;
    orc.id = 3;
    orc.name = "👹 ORC";
    orc.element = Element::FIRE;
    orc.rarity = EnemyRarity::RARE;
    orc.health = 80;
    orc.maxHealth = 80;
    orc.attack = 25;
    orc.defense = 10;
    orc.level = 2;
    orc.expReward = 50;
    orc.luminescenceReward = 100;
    allEnemies.push_back(orc);
    
    Enemy iceWolf;
    iceWolf.id = 4;
    iceWolf.name = "🐺 ICE WOLF";
    iceWolf.element = Element::WATER;
    iceWolf.rarity = EnemyRarity::RARE;
    iceWolf.health = 70;
    iceWolf.maxHealth = 70;
    iceWolf.attack = 20;
    iceWolf.defense = 12;
    iceWolf.level = 2;
    iceWolf.expReward = 45;
    iceWolf.luminescenceReward = 90;
    allEnemies.push_back(iceWolf);
    
    // Epic Enemies (15% spawn chance)
    Enemy fireDragon;
    fireDragon.id = 5;
    fireDragon.name = "🐉 FIRE DRAGON";
    fireDragon.element = Element::FIRE;
    fireDragon.rarity = EnemyRarity::EPIC;
    fireDragon.health = 150;
    fireDragon.maxHealth = 150;
    fireDragon.attack = 35;
    fireDragon.defense = 15;
    fireDragon.level = 3;
    fireDragon.expReward = 100;
    fireDragon.luminescenceReward = 200;
    allEnemies.push_back(fireDragon);
    
    // Legendary Enemies (5% spawn chance)
    Enemy ancientLich;
    ancientLich.id = 6;
    ancientLich.name = "💀 ANCIENT LICH";
    ancientLich.element = Element::DARK;
    ancientLich.rarity = EnemyRarity::LEGENDARY;
    ancientLich.health = 250;
    ancientLich.maxHealth = 250;
    ancientLich.attack = 50;
    ancientLich.defense = 25;
    ancientLich.level = 5;
    ancientLich.expReward = 250;
    ancientLich.luminescenceReward = 500;
    allEnemies.push_back(ancientLich);
    
    std::cout << "Loaded " << allEnemies.size() << " enemies" << std::endl;
}

Enemy EnemyManager::getRandomEnemy(int playerLevel) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    int roll = dis(gen);
    
    Enemy selectedEnemy;
    
    if (roll <= 50) {
        // Common enemy
        std::uniform_int_distribution<> commonDis(0, 1);
        selectedEnemy = allEnemies[commonDis(gen)];
    } else if (roll <= 80) {
        // Rare enemy
        std::uniform_int_distribution<> rareDis(2, 3);
        selectedEnemy = allEnemies[rareDis(gen)];
    } else if (roll <= 95) {
        // Epic enemy
        selectedEnemy = allEnemies[4];
    } else {
        // Legendary enemy
        selectedEnemy = allEnemies[5];
    }
    
    // Scale enemy stats based on player level
    selectedEnemy.health += (playerLevel - 1) * 10;
    selectedEnemy.maxHealth = selectedEnemy.health;
    selectedEnemy.attack += (playerLevel - 1) * 3;
    selectedEnemy.defense += (playerLevel - 1) * 2;
    selectedEnemy.level = playerLevel;
    
    return selectedEnemy;
}

Enemy EnemyManager::getEnemyById(int id) {
    for (const auto& enemy : allEnemies) {
        if (enemy.id == id) return enemy;
    }
    return Enemy();
}

std::vector<Enemy> EnemyManager::getEnemiesByRarity(EnemyRarity rarity) {
    std::vector<Enemy> result;
    for (const auto& enemy : allEnemies) {
        if (enemy.rarity == rarity) {
            result.push_back(enemy);
        }
    }
    return result;
}
