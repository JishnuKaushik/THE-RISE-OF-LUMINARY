#include "ElementSystem.hpp"
#include <iostream>

float ElementSystem::weaknessChart[6][6];

ElementSystem::ElementSystem() {
    initializeChart();
}

ElementSystem::~ElementSystem() {
}

void ElementSystem::initializeChart() {
    // Initialize all to 1.0 (normal damage)
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            weaknessChart[i][j] = 1.0f;
        }
    }
    
    // FIRE (index 0) strong against NATURE (3), weak against WATER (1)
    weaknessChart[0][3] = 2.0f;  // Fire vs Nature = Super effective
    weaknessChart[0][1] = 0.5f;  // Fire vs Water = Not effective
    
    // WATER (index 1) strong against FIRE (0), weak against LIGHTNING (2)
    weaknessChart[1][0] = 2.0f;  // Water vs Fire = Super effective
    weaknessChart[1][2] = 0.5f;  // Water vs Lightning = Not effective
    
    // LIGHTNING (index 2) strong against WATER (1), weak against NATURE (3)
    weaknessChart[2][1] = 2.0f;  // Lightning vs Water = Super effective
    weaknessChart[2][3] = 0.5f;  // Lightning vs Nature = Not effective
    
    // NATURE (index 3) strong against LIGHTNING (2), weak against FIRE (0)
    weaknessChart[3][2] = 2.0f;  // Nature vs Lightning = Super effective
    weaknessChart[3][0] = 0.5f;  // Nature vs Fire = Not effective
    
    // DARK (index 4) strong against LIGHT (5), weak against LIGHT (5)
    weaknessChart[4][5] = 2.0f;  // Dark vs Light = Super effective
    weaknessChart[5][4] = 2.0f;  // Light vs Dark = Super effective
}

float ElementSystem::getElementMultiplier(Element attacker, Element defender) {
    int attIndex = static_cast<int>(attacker);
    int defIndex = static_cast<int>(defender);
    
    if (attIndex >= 0 && attIndex < 6 && defIndex >= 0 && defIndex < 6) {
        return weaknessChart[attIndex][defIndex];
    }
    return 1.0f;
}

int ElementSystem::calculateDamage(int baseDamage, Element attacker, Element defender, int enemyDefense) {
    float multiplier = getElementMultiplier(attacker, defender);
    int damage = baseDamage - (enemyDefense / 2);
    if (damage < 5) damage = 5;
    damage = static_cast<int>(damage * multiplier);
    return damage;
}

std::string ElementSystem::getEffectivenessMessage(float multiplier) {
    if (multiplier > 1.0f) {
        return "⭐ SUPER EFFECTIVE! 2x damage!";
    } else if (multiplier < 1.0f) {
        return "⚠️ NOT VERY EFFECTIVE... half damage";
    }
    return "";
}
