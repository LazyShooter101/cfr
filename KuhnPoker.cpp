#include <iostream>
#include <random>
#include <array>
#include <string>
#include <map>
#include <algorithm>
#include <time.h>

class KuhnPokerCFR {
    static const int NUM_ACTIONS = 2;
    static const int PASS = 0, BET = 1;
public:
   
    KuhnPokerCFR() {
        srand(5);
    }        

    // NODE CLASS
private:
    class Node {
    public:
        std::string infoSet;
        std::array<double, NUM_ACTIONS> regretSum;        
        std::array<double, NUM_ACTIONS> strategy {0.0};
        std::array<double, NUM_ACTIONS> strategySum {0.0};

        std::array<double, NUM_ACTIONS> getAverageStrategy() {
            std::array<double, NUM_ACTIONS> avgStrategy;
            double normalisingSum = 0.0;
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
            std::string res = infoSet + ": [" + std::to_string(avgStrategy[0]) + ", " + std::to_string(avgStrategy[1]) + "]";
            return res;
        }
    };

    std::map<std::string, Node *> m_nodeMap;

// KUHN POKER METHODS:
public:
    void train(int iterations) {
        std::array<int, 3> cards {1, 2, 3};
        double util = 0.0;
        for (int i=0; i<iterations; i++) {
            if (i % 10000 == 0) std::cout << "Starting iteration " << i << ":\n";
            // shuffle cards
            std::random_shuffle(cards.begin(), cards.end());
            util += cfr(cards, "", 1.0, 1.0);
        }
        std::cout << "Average game value: " << util/iterations << "\nFinal Strategy:\n";
        for (auto &n : m_nodeMap) {
            std::cout << "\t" << n.second->toString() << "\n";
        }
    }

private:
    double cfr(std::array<int, 3> cards, std::string history, double p0, double p1) {
        int turnIndex = history.length();
        int player = turnIndex % 2; // player 1 for even turns, player 2 for odd. turns start at 0
        int opponent = 1 - player;
        // Return payoff for terminal states
        if (turnIndex > 1) {
            bool terminalPass = history.back() == 'p';
            bool doubleBet = history.substr(turnIndex-2) == "bb";
            bool isPlayerCardHigher = cards[player] > cards[opponent];
            if (terminalPass) {
                if (history.substr(turnIndex-2) == "pp") {
                    return isPlayerCardHigher ? 1 : -1;
                } else {
                    return 1;
                }
            } else if (doubleBet) {
                return isPlayerCardHigher ? 2 : -2;
            }
        }

        // create info set
        std::string infoSet = std::to_string(cards[player]) + history;
        
        // get node or create it if nonexistant
        Node *node;
        if (m_nodeMap.find(infoSet) == m_nodeMap.end()) {
            // node doesn't exist yet
            node = new Node();
            node->infoSet = infoSet;
            m_nodeMap[infoSet] = node;
        } else
            node = m_nodeMap[infoSet];
        
        // for each action, recursively call cfr with additional history and 
        // probability
        std::array<double, NUM_ACTIONS> strategy = node->getStrategy(player == 0 ? p0 : p1);
        std::array<double, NUM_ACTIONS> util {0.0, 0.0};
        double nodeUtil = 0.0;
        for (int a=0; a<NUM_ACTIONS; a++) {
            std::string nextHistory = history + (a==0 ? "p" : "b");
            if (player == 0)
                util[a] = -cfr(cards, nextHistory, p0*strategy[a], p1);
            else
                util[a] = -cfr(cards, nextHistory, p0, p1*strategy[a]);
            nodeUtil += strategy[a] * util[a];
        }
        
        // for each action, compute and accumulate counterfactual regret
        for (int a=0; a<NUM_ACTIONS; a++) {
            double regret = util[a] - nodeUtil;
            node->regretSum[a] += (player == 0 ? p1 : p0) * regret;
        }
        
        return nodeUtil; 
    }

};

int main() {
    KuhnPokerCFR trainer = KuhnPokerCFR();
    trainer.train(1000000);

    return 0;
}