#include <iostream>
#include <random>
#include <array>

class RockPaperScissorsCFR {

public:
    static const int ROCK = 0, PAPER = 1, SCISSORS = 2;
    static const int NUM_ACTIONS = 3;

    std::array<double, NUM_ACTIONS> m_regretSum {0};
    std::array<double, NUM_ACTIONS> m_strategySum {0};
    std::array<double, NUM_ACTIONS> m_oppRegretSum {0};
    std::array<double, NUM_ACTIONS> m_oppStrategySum {0};

    RockPaperScissorsCFR() {
        std::cout << "Initialising RockPaperScissorsCFR instance\n";
    }

    int getAction(std::array<double, NUM_ACTIONS> strategy) {
        double r = (double)rand()/RAND_MAX;
        int a = 0;
        double cumulativeProbability = 0.0;
        while (a < NUM_ACTIONS - 1) {
            cumulativeProbability += strategy[a];
            if (r < cumulativeProbability)
                break;
            a++;
        }
        return a;
    }

    void train(int iterations) {
        std::array<double, NUM_ACTIONS> actionUtility {0};
        for (int i = 0; i < iterations; i++) {
            std::array<double, NUM_ACTIONS> myStrategy = getStrategy(m_regretSum);
            std::array<double, NUM_ACTIONS> oppStrategy = getStrategy(m_oppRegretSum);
            int myAction = getAction(myStrategy);
            int oppAction = getAction(oppStrategy);

            actionUtility[oppAction] = 0; // doing the same as opp. has value 0
            switch (oppAction) {
                case ROCK:
                    actionUtility[PAPER] = 1;
                    actionUtility[SCISSORS] = -1;
                    break;
                case PAPER:
                    actionUtility[SCISSORS] = 1;
                    actionUtility[ROCK] = -1;
                    break;
                case SCISSORS:
                    actionUtility[ROCK] = 1;
                    actionUtility[PAPER] = -1;
                    break;
            }

            for (int a=0; a<NUM_ACTIONS; ++a) {
                m_regretSum[a] += actionUtility[a] - actionUtility[myAction];
                m_strategySum[a] += myStrategy[a];
            }

            actionUtility[myAction] = 0; // doing the same as opp. has value 0
            switch (myAction) {
                case ROCK:
                    actionUtility[PAPER] = 1;
                    actionUtility[SCISSORS] = -1;
                    break;
                case PAPER:
                    actionUtility[SCISSORS] = 1;
                    actionUtility[ROCK] = -1;
                    break;
                case SCISSORS:
                    actionUtility[ROCK] = 1;
                    actionUtility[PAPER] = -1;
                    break;
            }

            for (int a=0; a<NUM_ACTIONS; ++a) {
                m_oppRegretSum[a] += actionUtility[a] - actionUtility[oppAction];
                m_oppStrategySum[a] += oppStrategy[a];
            }
        }
    }

    std::array<double, NUM_ACTIONS> getAverageStrategy() {
        std::array<double, NUM_ACTIONS> avgStrategy {0};
        double normalizingSum = 0.0;
        for (int a=0; a<NUM_ACTIONS; a++) {
            normalizingSum += m_strategySum[a];
        }
        for (int a=0; a<NUM_ACTIONS; a++) {
            if (normalizingSum > 0) {
                avgStrategy[a] = m_strategySum[a] / normalizingSum;
            } else {
                avgStrategy[a] = 1.0 / NUM_ACTIONS;
            }
        }
        return avgStrategy;
    }

private:

    std::array<double, NUM_ACTIONS> getStrategy(std::array<double, NUM_ACTIONS> regretSum) {
        double normalizingSum = 0.0;
        std::array<double, NUM_ACTIONS> strategy;
        for (int a=0; a<NUM_ACTIONS; ++a) {
            strategy[a] = regretSum[a] > 0 ? regretSum[a] : 0;
            normalizingSum += strategy[a];
        }
        for (int a=0; a<NUM_ACTIONS; ++a) {
            if (normalizingSum > 0) {
                strategy[a] /= normalizingSum;
            } else {
                strategy[a] = 1.0 / NUM_ACTIONS;
            }
        }
        return strategy;
    }
};


int main() {
    std::cout << "Starting\n";

    RockPaperScissorsCFR cfr_game;
    cfr_game.train(10000000);
    auto finalStrategy = cfr_game.getAverageStrategy();
    std::cout << "Final Strategy: (";
    for (int a=0; a<finalStrategy.size(); a++) {
        if (a)
            std::cout << ", ";
        std::cout << finalStrategy[a];
    }
    std::cout << ")\n";

    return 0;    
}