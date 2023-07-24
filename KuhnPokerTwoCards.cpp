#include <iostream>
#include <random>
#include <array>
#include <string>
#include <map>
#include <algorithm>
#include <time.h>

class KuhnPokerTwoCardsCFR {
    static const int NUM_ACTIONS = 3, NUM_CARDS = 4*4;
    static const int PASS = 0, SMALL_BET = 1, BIG_BET = 2;

private:
    class Node {
    public:
        std::string infoSet;
        std::array<double, NUM_ACTIONS> regretSum {0.0};
        std::array<double, NUM_ACTIONS> strategy {0.0};
        std::array<double, NUM_ACTIONS> strategySum {0.0};

        std::array<double, NUM_ACTIONS> getAverageStrategy() {
            std::array<double, NUM_ACTIONS> avgStrategy;
            double normalisingSum;
            for (int a=0; a<NUM_ACTIONS; a++)
                normalisingSum += strategySum[a];
            for (int a=0; a<NUM_ACTIONS; a++) {
                if (normalisingSum > 0)
                    avgStrategy[a] = strategySum[a] / normalisingSum;
                else
                    avgStrategy[a] = 1.0 / NUM_ACTIONS;
            }
            return avgStrategy;
        }

        std::array<double, NUM_ACTIONS> getStrategy(double realisationWeight) {
            double normalisingSum = 0.0;
            for (int a=0; a<NUM_ACTIONS; a++) {
                strategy[a] = regretSum[a] > 0 ? regretSum[a] : 0;
                normalisingSum += strategy[a];
            }
            for (int a=0; a<NUM_ACTIONS; a++) {
                if (normalisingSum > 0)
                    strategy[a] /= normalisingSum;
                else
                    strategy[a] = 1.0 / NUM_ACTIONS;
                strategySum[a] += realisationWeight * strategy[a];
            }
            return strategy;
        }

        std::string toString() {
            std::array<double, NUM_ACTIONS> avgStrategy = getAverageStrategy();
            std::string res = infoSet + ": [" + std::to_string(avgStrategy[0]) + ", " + std::to_string(avgStrategy[1]) + ", " + std::to_string(avgStrategy[2]) + "]";
            return res;
        }
    };

    std::map<std::string, Node *> m_nodeMap;

public:
    void train(int iterations) {
        std::array<int, NUM_CARDS> cards {1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4};
        double util = 0.0;
        int n_doubles = 0;
        for (int i=0; i<iterations; i++) {
            if (i % 1000000 == 0) 
            std::cout << "Training " << 100.0*i/(double)iterations << "\% done\n";
            // shuffle cards
            std::random_shuffle(cards.begin(), cards.end());
            if (cards[0] == cards[1])
                n_doubles++;
            util += cfr(cards, "", 1.0, 1.0);
        }
        std::cout << "Chance for pair: " << 100.0*n_doubles/(double)iterations << "\%\n";
        std::cout << "Average game value: " << util/iterations << "\nFinal Strategy:\n";
        for (auto &n : m_nodeMap) {
            std::cout << "\t" << n.second->toString() << "\n";
        }
    }

private:
    double cfr(std::array<int, NUM_CARDS> cards, std::string history, double p0, double p1) {
        // std::cout << "CFR(" << history << ")\n";
        
        int turnIndex = history.length();
        int player = turnIndex % 2;
        int opponent = 1-player;
        int playerCard1 = cards[2*player];
        int playerCard2 = cards[2*player+1];
        if (playerCard1 > playerCard2) { // ensure sorted
            int temp = playerCard1;
            playerCard1 = playerCard2;
            playerCard2 = temp;
        }
        int opponentCard1 = cards[2*opponent];
        int opponentCard2 = cards[2*opponent+1];
        if (opponentCard1 > opponentCard2) { // ensure sorted
            int temp = opponentCard1;
            opponentCard1 = opponentCard2;
            opponentCard2 = temp;
        }

        // Return payoff for terminal states
        if (turnIndex >= 2) {
            std::string lastTwo = history.substr(turnIndex - 2);
            // get winning card data, 0 if opponent wins, 1 if player wins, -1 if draw
            int winningCardsData = arePlayerCardsHigher(playerCard1, playerCard2, opponentCard1, opponentCard2);
            // winning score: 0 if draw, 1 if player wins, -1 if opponent wins
            int winningScore = winningCardsData == -1 ? 0 : winningCardsData*2 - 1;
            if (lastTwo == "pp")
                return winningScore;
            else if (lastTwo == "bb")
                return winningScore * 2.0;
            else if (lastTwo == "BB")
                return winningScore * 4.0;
            else if (lastTwo == "bp")
                return 1.0;
            else if (lastTwo == "Bp" || lastTwo == "Bb") {
                // we now must determine if the player who folds after the B
                // had previously bet small before.
                if (turnIndex == 2) // they can't have done so
                    return 1.0;
                if (turnIndex == 4) // they must have done so (pbBp)
                    return 2.0;
                if (history.substr(turnIndex - 3, 1) == "p") // they passed previously
                    return 1.0;
                else
                    return 2.0;
            }
        }

        // Create info set
        std::string infoSet = std::to_string(playerCard1) + std::to_string(playerCard2) + history;

        // get node or create it if nonexistant
        Node *node;
        if (m_nodeMap.find(infoSet) == m_nodeMap.end()) {
            // node doesn't exist yet
            node = new Node();
            node->infoSet = infoSet;
            m_nodeMap[infoSet] = node;
            // std::cout << "Creating Node(" << infoSet << ")";
        } else
            node = m_nodeMap[infoSet];

        // for each action recursively call cfr
        // with additional history and probability
        std::array<double, NUM_ACTIONS> strategy = node->getStrategy(player == 0 ? p0 : p1);
        std::array<double, NUM_ACTIONS> util {0.0};
        double nodeUtil = 0.0;
        for (int a=0; a<NUM_ACTIONS; a++) {
            std::string nextHistory = history + "";
            switch (a) {
                case PASS: nextHistory += "p"; break;
                case SMALL_BET: nextHistory += "b"; break;
                case BIG_BET: nextHistory += "B"; break;
            }
            if (player == 0)
                util[a] = -cfr(cards, nextHistory, p0*strategy[a], p1);
            else
                util[a] = -cfr(cards, nextHistory, p0, p1*strategy[a]);
            // std::cout << "(" << history << ") calling CFR(" << nextHistory << "), got " << util[a] << "\n";
            nodeUtil += strategy[a] * util[a];
        }

        // for each action, compute and accumulate cfr
        for (int a=0; a<NUM_ACTIONS; a++) {
            double regret = util[a] - nodeUtil;
            node->regretSum[a] += (player == 0 ? p1 : p0) * regret;
            // std::cout << "Node(" << infoSet << ") regret[" << a << "], adding " << (player == 0 ? p1 : p0) * regret << ", -> " << m_nodeMap[infoSet]->regretSum[a] << "\n";
        }

        return nodeUtil;
    }

    int arePlayerCardsHigher(int playerCard1, int playerCard2, int opponentCard1, int opponentCard2) {
        bool playerHasPair = playerCard1 == playerCard2;
        bool opponentHasPair = opponentCard1 == opponentCard2;
        if (playerHasPair) {
            if (opponentHasPair) {
                if (playerCard1 == opponentCard1)
                    return -1;
                return playerCard1 > opponentCard1;
            }
            return 1;
        }
        if (opponentHasPair)
            return 0;

        int playerBestCard = std::max(playerCard1, playerCard2);
        int opponentBestCard = std::max(opponentCard1, opponentCard2);
        if (playerBestCard > opponentBestCard)
            return 1;
        if (opponentBestCard > playerBestCard)
            return 0;

        int playerSecondBestCard = std::min(playerCard1, playerCard2);
        int opponentSecondBestCard = std::min(opponentCard1, opponentCard2);
        if (playerSecondBestCard > opponentSecondBestCard)
            return 1;
        if (opponentSecondBestCard > playerSecondBestCard)
            return 0;
        // it's a tie!
        return -1;
    }
};

int main() {
    srand(9);
    
    KuhnPokerTwoCardsCFR trainer = KuhnPokerTwoCardsCFR();
    trainer.train(100000000);

    return 0;
}