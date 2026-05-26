#pragma once
#include "Character.hpp"

class ElementSystem {
private:
    static float weaknessChart[6][6];
    static void initializeChart();
    
public:
    ElementSystem();
    ~ElementSystem();
    
    static float getElementMultiplier(Element attacker, Element defender);
    static int calculateDamage(int baseDamage, Element attacker, Element defender, int enemyDefense);
    static std::string getEffectivenessMessage(float multiplier);
};
