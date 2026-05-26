#include "Card.hpp"
#include <iostream>
#include <algorithm>
#include <random>

std::string Card::getRarityString() const {
    switch (rarity) {
        case CardRarity::COMMON: return "COMMON";
        case CardRarity::RARE: return "RARE";
        case CardRarity::EPIC: return "EPIC";
        case CardRarity::LEGENDARY: return "LEGENDARY";
        default: return "COMMON";
    }
}

std::string Card::getElementString() const {
    switch (element) {
        case CardElement::FIRE: return "🔥 FIRE";
        case CardElement::WATER: return "💧 WATER";
        case CardElement::LIGHTNING: return "⚡ LIGHTNING";
        case CardElement::NATURE: return "🌿 NATURE";
        case CardElement::DARK: return "🌑 DARK";
        case CardElement::LIGHT: return "✨ LIGHT";
        default: return "⚪ NEUTRAL";
    }
}

CardManager::CardManager() : maxHandSize(5) {
    initialize();
}

CardManager::~CardManager() {
}

void CardManager::initialize() {
    loadDefaultCards();
    playerDeck = allCards;
    shuffleDeck();
    drawInitialHand();
}

void CardManager::loadDefaultCards() {
    // Card 1: Fireball
    Card fireball;
    fireball.id = 1;
    fireball.name = "🔥 FIREBALL";
    fireball.rarity = CardRarity::COMMON;
    fireball.element = CardElement::FIRE;
    fireball.damage = 20;
    fireball.healAmount = 0;
    fireball.manaCost = 1;
    fireball.description = "Deal 20 damage to enemy";
    fireball.specialEffect = "30% chance to burn";
    fireball.isOwned = true;
    allCards.push_back(fireball);
    
    // Card 2: Water Shield
    Card waterShield;
    waterShield.id = 2;
    waterShield.name = "💧 WATER SHIELD";
    waterShield.rarity = CardRarity::COMMON;
    waterShield.element = CardElement::WATER;
    waterShield.damage = 0;
    waterShield.healAmount = 15;
    waterShield.manaCost = 1;
    waterShield.description = "Heal 15 HP";
    waterShield.specialEffect = "Removes burn";
    waterShield.isOwned = true;
    allCards.push_back(waterShield);
    
    // Card 3: Lightning Strike
    Card lightningStrike;
    lightningStrike.id = 3;
    lightningStrike.name = "⚡ LIGHTNING STRIKE";
    lightningStrike.rarity = CardRarity::COMMON;
    lightningStrike.element = CardElement::LIGHTNING;
    lightningStrike.damage = 25;
    lightningStrike.healAmount = 0;
    lightningStrike.manaCost = 1;
    lightningStrike.description = "Deal 25 damage";
    lightningStrike.specialEffect = "20% chance to stun";
    lightningStrike.isOwned = true;
    allCards.push_back(lightningStrike);
    
    // Card 4: Nature's Blessing
    Card natureBless;
    natureBless.id = 4;
    natureBless.name = "🌿 NATURE'S BLESSING";
    natureBless.rarity = CardRarity::RARE;
    natureBless.element = CardElement::NATURE;
    natureBless.damage = 0;
    natureBless.healAmount = 10;
    natureBless.manaCost = 1;
    natureBless.description = "Heal 10 HP over 3 turns";
    natureBless.specialEffect = "Regeneration effect";
    natureBless.isOwned = false;
    allCards.push_back(natureBless);
    
    // Card 5: Dark Pulse
    Card darkPulse;
    darkPulse.id = 5;
    darkPulse.name = "🌑 DARK PULSE";
    darkPulse.rarity = CardRarity::RARE;
    darkPulse.element = CardElement::DARK;
    darkPulse.damage = 30;
    darkPulse.healAmount = 0;
    darkPulse.manaCost = 2;
    darkPulse.description = "Deal 30 damage";
    darkPulse.specialEffect = "Lifesteal 10 HP";
    darkPulse.isOwned = false;
    allCards.push_back(darkPulse);
    
    // Card 6: Holy Light
    Card holyLight;
    holyLight.id = 6;
    holyLight.name = "✨ HOLY LIGHT";
    holyLight.rarity = CardRarity::EPIC;
    holyLight.element = CardElement::LIGHT;
    holyLight.damage = 40;
    holyLight.healAmount = 20;
    holyLight.manaCost = 2;
    holyLight.description = "Deal 40 damage and heal 20 HP";
    holyLight.specialEffect = "Double damage against dark";
    holyLight.isOwned = false;
    allCards.push_back(holyLight);
    
    // Card 7: Meteor Storm
    Card meteor;
    meteor.id = 7;
    meteor.name = "☄️ METEOR STORM";
    meteor.rarity = CardRarity::LEGENDARY;
    meteor.element = CardElement::FIRE;
    meteor.damage = 60;
    meteor.healAmount = 0;
    meteor.manaCost = 3;
    meteor.description = "Deal 60 damage to all enemies";
    meteor.specialEffect = "100% burn chance";
    meteor.isOwned = false;
    allCards.push_back(meteor);
    
    std::cout << "Loaded " << allCards.size() << " cards" << std::endl;
}

void CardManager::shuffleDeck() {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(playerDeck.begin(), playerDeck.end(), g);
}

void CardManager::drawCard() {
    if (playerDeck.empty()) return;
    if (playerHand.size() >= maxHandSize) return;
    
    playerHand.push_back(playerDeck.back());
    playerDeck.pop_back();
}

void CardManager::drawInitialHand() {
    playerHand.clear();
    for (int i = 0; i < 3; i++) {
        drawCard();
    }
}

void CardManager::addCardToDeck(int cardId) {
    for (auto& card : allCards) {
        if (card.id == cardId && !card.isOwned) {
            card.isOwned = true;
            playerDeck.push_back(card);
            std::cout << "Added card to deck: " << card.name << std::endl;
            break;
        }
    }
}

bool CardManager::playCard(int cardIndex) {
    if (cardIndex < 0 || cardIndex >= (int)playerHand.size()) return false;
    
    Card playedCard = playerHand[cardIndex];
    playerHand.erase(playerHand.begin() + cardIndex);
    drawCard();
    
    std::cout << "Played card: " << playedCard.name << std::endl;
    return true;
}

Card* CardManager::getCardById(int id) {
    for (auto& card : allCards) {
        if (card.id == id) return &card;
    }
    return nullptr;
}

void CardManager::resetHand() {
    for (auto& card : playerHand) {
        playerDeck.push_back(card);
    }
    playerHand.clear();
    shuffleDeck();
    drawInitialHand();
}
