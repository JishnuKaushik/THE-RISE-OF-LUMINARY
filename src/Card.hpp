#pragma once
#include <string>
#include <vector>

enum class CardRarity {
    COMMON,
    RARE,
    EPIC,
    LEGENDARY
};

enum class CardElement {
    FIRE,
    WATER,
    LIGHTNING,
    NATURE,
    DARK,
    LIGHT,
    NEUTRAL
};

struct Card {
    int id;
    std::string name;
    CardRarity rarity;
    CardElement element;
    int damage;
    int healAmount;
    int manaCost;
    std::string description;
    std::string specialEffect;
    bool isOwned;
    
    Card() : id(0), rarity(CardRarity::COMMON), element(CardElement::NEUTRAL),
             damage(0), healAmount(0), manaCost(1), isOwned(false) {}
    
    std::string getRarityString() const;
    std::string getElementString() const;
};

class CardManager {
private:
    std::vector<Card> allCards;
    std::vector<Card> playerDeck;
    std::vector<Card> playerHand;
    int maxHandSize;
    
    void loadDefaultCards();
    void shuffleDeck();
    
public:
    CardManager();
    ~CardManager();
    
    void initialize();
    void drawCard();
    void drawInitialHand();
    void addCardToDeck(int cardId);
    bool playCard(int cardIndex);
    std::vector<Card> getCurrentHand() const { return playerHand; }
    Card* getCardById(int id);
    int getDeckSize() const { return playerDeck.size(); }
    int getHandSize() const { return playerHand.size(); }
    void resetHand();
};
