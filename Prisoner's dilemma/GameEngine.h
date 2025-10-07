#include "Strategy.h"
#include <iostream>
#include <map>
using ScorePair = std::pair<double, double>;
using StrategyPtr = std::unique_ptr<Strategy>;

//
const std::map<std::pair<Move, Move>, ScorePair> PAYOFF_MATRIX = {
    {{Move::Cooperate, Move::Cooperate}, {3.0, 3.0}}, //
    {{Move::Cooperate, Move::Defect},    {0.0, 5.0}}, // 
    {{Move::Defect, Move::Cooperate},    {5.0, 0.0}}, // 
    {{Move::Defect, Move::Defect},      {1.0, 1.0}}  // 
};

inline std::string moveToString(Move m) {
    return m == Move::Cooperate ? "C (Cooperate)" : "D (Defect)";
}

class Simulator {
private:
    std::vector<double> payoff_config;
    double getScore(Move m1, Move m2) const {
        if (m1 == Move::Defect && m2 == Move::Cooperate) { return payoff_config[0]; }//T
        if (m1 == Move::Cooperate && m2 == Move::Cooperate) { return payoff_config[1]; }//R
        if (m1 == Move::Defect && m2 == Move::Defect) { return payoff_config[2]; }//R
        if (m1 == Move::Cooperate && m2 == Move::Defect) { return payoff_config[3]; }//R
        return 0.0; 

    }
public:
    Simulator(const std::vector<double>& config) : payoff_config(config) {}

    ScorePair runGame(const StrategyPtr& p1, const StrategyPtr& p2, int rounds) const {
        History history;
        double score1 = 0.0;
        double score2 = 0.0;
        for (int i = 1; i <= rounds; ++i) {
            Move move1 = p1->decide(history);
            Move move2 = p2->decide(history);
            double round_score1 = getScore(move1, move2);
            double round_score2 = getScore(move2, move1);
            score1 += round_score1;
            score2 += round_score2;
            history.push_back({ move1, move2 });
        }
        return { score1, score2 };
    };
    std::map<std::string, double> runTournament(const std::vector<StrategyPtr>& strategies, int rounds, int repeats) const {
        std::map<std::string, double> totalScores;
        std::map<std::string, int> matchCounts;
        for (const auto& s : strategies)
        {
            totalScores[s->getName()] = 0.0;
            matchCounts[s->getName()] = 0;
        }
        // 循环赛：每个策略两两对战
        for (size_t i = 0; i < strategies.size(); ++i)
        {
            for (size_t j = i; j < strategies.size(); ++j)
            {
                const auto& p1 = strategies[i];
                const auto& p2 = strategies[j];
                double cumulative_score1 = 0.0;
                double cumulative_score2 = 0.0;
                for (int r = 0; r < repeats; ++r)
                {
                    ScorePair scores = runGame(p1, p2, rounds); //p1,p2ÖØ¸´roundsÂÖµÃµ½Á½¸ö·ÖÊý
                    cumulative_score1 += scores.first;
                    cumulative_score2 += scores.second;
                }
                totalScores[p1->getName()] += cumulative_score1; //p1和p2在经过了repeats次重复，每次对战rounds轮后，获得的总分(某一场具体对战 的平均得分)
                totalScores[p2->getName()] += cumulative_score2;
                matchCounts[p1->getName()] += repeats;
				matchCounts[p2->getName()] += repeats;
                std::cout << "  - " << p1->getName() << " vs " << p2->getName()
                    << " Avg Scores: P1=" << cumulative_score1 / repeats
                    << ", P2=" << cumulative_score2 / repeats << "\n";
            }
        }
        ///循环赛结束后 的全局平均分
		std::map<std::string,double> avg_scores;
        for (const auto& [name, total] : totalScores) {
            if (matchCounts.at(name) > 0) {
                avg_scores[name] = total / matchCounts.at(name);
            }
        }
		return avg_scores;
    };
};

#pragma once