#include <iostream>
#include <random>
#include <array>
#include <set>
#include <map>

typedef std::array<int, 3> Action;
typedef std::map<Action, double> ActionDoubles;

class BlottoTrainer {
public:
    static const int S = 5;
    static const int N = 3;
    static int N_ACTIONS;
    std::set<Action> allActions; 

    ActionDoubles m_regretSum;
    ActionDoubles m_strategySum;
    ActionDoubles m_oppRegretSum;
    ActionDoubles m_oppStrategySum;

    BlottoTrainer() {
        // init actions
        Action a;
        int nAtThird;
        for (int nAtFirst = 0; nAtFirst <= S; nAtFirst++) {
            for (int nAtSecond = 0; nAtSecond+nAtFirst <= S; nAtSecond++) {
                nAtThird = S - nAtFirst - nAtSecond;
                a = {nAtFirst, nAtSecond, nAtThird};
                allActions.insert(a);
            }
        }
    }

    Action getAction(ActionDoubles strategy) {
        double r = (double)rand()/RAND_MAX;
        double cumulativeProb = 0.0;
        Action finalAction;
        for (Action a : allActions) {
            cumulativeProb += strategy[a];
            if (r < cumulativeProb) {
                finalAction = a;
                break;
            }
        }
        return finalAction;
    }

    void train(int iterations) {
        ActionDoubles actionUtility;
        ActionDoubles myStrategy, oppStrategy;
        for (int i=0; i<iterations; i++) {
            myStrategy = getStrategy(m_regretSum);
            oppStrategy = getStrategy(m_oppRegretSum);
            Action myAction = getAction(myStrategy);
            Action oppAction = getAction(oppStrategy);

            actionUtility = getActionUtility(oppAction);
            for (Action a : allActions) {
                m_regretSum[a] += actionUtility[a] - actionUtility[myAction];
                m_strategySum[a] += myStrategy[a];
            }

            actionUtility = getActionUtility(myAction);
            for (Action a : allActions) {
                m_oppRegretSum[a] += actionUtility[a] - actionUtility[oppAction];
                oppStrategy[a] += oppStrategy[a];
            }
        }
    }

    ActionDoubles getAverageStrategy() {
        return getStrategy(m_strategySum);
    }

private:

    ActionDoubles getStrategy(ActionDoubles regretSum) {
        double normalizingSum = 0.0;
        ActionDoubles strategy;
        for (Action a : allActions) {
            strategy[a] = regretSum[a] > 0 ? regretSum[a] : 0;
            normalizingSum += strategy[a];
        }
        for (Action a : allActions) {
            if (normalizingSum > 0) {
                strategy[a] /= normalizingSum;
            } else {
                strategy[a] = 1.0 / allActions.size();
            }
        }
        return strategy;
    }

    ActionDoubles getActionUtility(Action oppAction) {
        ActionDoubles actionUtilty;
        int nBattlesWon;
        int nBattlesOppWon;
        for (Action a : allActions) {
            nBattlesWon = 0;
            nBattlesOppWon = 0;
            for (int battle = 0; battle < N; battle++) {
                if (oppAction[battle] < a[battle])
                    nBattlesWon++;
                else if (oppAction[battle] > a[battle])
                    nBattlesOppWon++;
            }
            if (nBattlesWon > nBattlesOppWon)
                actionUtilty[a] = 1;
            else if (nBattlesWon < nBattlesOppWon)
                actionUtilty[a] = -1;
            else
                actionUtilty[a] = 0;
        }
        return actionUtilty;
    }

};


int main() {
    BlottoTrainer trainer = BlottoTrainer();
    trainer.train(1000000);
    ActionDoubles finalStrategy = trainer.getAverageStrategy();
    for (Action a : trainer.allActions) {
        std::cout << a[0] << " " << a[1] << " " << a[2] << " : " << finalStrategy[a] << "\n";
    }

    return 0;
}