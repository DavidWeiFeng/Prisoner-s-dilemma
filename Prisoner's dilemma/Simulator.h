#ifndef SIMULATOR_H
#define SIMULATOR_H
#include <sstream>
#include "Strategy.h"
#include <iostream>
#include <iomanip>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <random>
#include <tabulate/table.hpp>
#include <numeric>

using ScorePair = std::pair<double, double>;
using StrategyPtr = std::unique_ptr<Strategy>;

inline std::string moveToString(Move m) {
    return m == Move::Cooperate ? "C (Cooperate)" : "D (Defect)";
}

// Structure to hold statistical information about scores
struct ScoreStats {
    double mean;
    double stdev;
    double ci_lower;
    double ci_upper;
    int n_samples;

    ScoreStats() : mean(0.0), stdev(0.0), ci_lower(0.0), ci_upper(0.0), n_samples(0) {}
    
    ScoreStats(double m, double sd, double ci_low, double ci_high, int n)
        : mean(m), stdev(sd), ci_lower(ci_low), ci_upper(ci_high), n_samples(n) {}
};

class Simulator {
private:
    std::vector<double> payoff_config;
    double noise_level;  // Current noise level

    double getScore(Move m1, Move m2) const {
        if (m1 == Move::Defect && m2 == Move::Cooperate) { return payoff_config[0]; }//T
        if (m1 == Move::Cooperate && m2 == Move::Cooperate) { return payoff_config[1]; }//R
        if (m1 == Move::Defect && m2 == Move::Defect) { return payoff_config[2]; }//P
        if (m1 == Move::Cooperate && m2 == Move::Defect) { return payoff_config[3]; }//S
        return 0.0;
    }

public:
    explicit Simulator(const std::vector<double>& config, double noise = 0.0) 
        : payoff_config(config), noise_level(noise) {}

    // Set noise level
    void setNoise(double epsilon) { 
        noise_level = epsilon; 
    }

    double getNoise() const { 
        return noise_level; 
    }

    // Run a single match, considering noise
    ScorePair runGame(const StrategyPtr& p1, const StrategyPtr& p2, int rounds) const {
        History history1;// player1's perspective: {my move, opponent's move}
        History history2;
        double score1 = 0.0;
        double score2 = 0.0;
        
        for (int i = 1; i <= rounds; ++i) {
            // Use decideWithNoise method to get decision with noise
            Move move1 = p1->decideWithNoise(history1);
            Move move2 = p2->decideWithNoise(history2);
            
            double round_score1 = getScore(move1, move2);
            double round_score2 = getScore(move2, move1);
            score1 += round_score1;
            score2 += round_score2;
            // update history, from each player's perspective
            history1.push_back({ move1, move2 });  // player1: I play move1, opponent plays move2
            history2.push_back({ move2, move1 });  // player2: I play move2, opponent plays move1
        }

        // SCB: If complexity cost is enabled, deduct it from final score
        if (Strategy::isSCBEnabled()) {
            double cost1 = p1->getComplexity() * Strategy::getSCBCostFactor() * rounds;
            double cost2 = p2->getComplexity() * Strategy::getSCBCostFactor() * rounds;
            score1 -= cost1;
            score2 -= cost2;
        }

        return { score1, score2 };
    }
    // Calculate mean and standard deviation from a vector of scores
    inline ScoreStats calculateStats(const std::vector<double>& scores) const {
        ScoreStats stats;
        stats.n_samples = scores.size();
        if (scores.empty()) return stats;

        double sum = std::accumulate(scores.begin(), scores.end(), 0.0);
        stats.mean = sum / stats.n_samples;

        if (stats.n_samples > 1) {
            double variance = 0.0;
            for (double s : scores)
                variance += (s - stats.mean) * (s - stats.mean);
            variance /= (stats.n_samples - 1);
            stats.stdev = std::sqrt(variance);

            double margin = 1.96 * (stats.stdev / std::sqrt(stats.n_samples));
            stats.ci_lower = stats.mean - margin;
            stats.ci_upper = stats.mean + margin;
        }
        else {
            stats.stdev = 0.0;
            stats.ci_lower = stats.ci_upper = stats.mean;
        }

        return stats;
    }
    // Standard tournament with confidence intervals
    // Returns a pair: first is strategy statistics results, second is match matrix (for printing)
    std::pair<std::map<std::string, ScoreStats>, std::vector<std::vector<ScorePair>>> 
    runTournament(const std::vector<StrategyPtr>& strategies, int rounds, int repeats) const {
		std::map<std::string, std::vector<double>> allScores; // collect all scores for each strategy
        // Initialize
        for (const auto& s : strategies) {
            allScores[s->getName()] = std::vector<double>();
        }

        // Store detailed match results for table display
        int N = strategies.size();
        std::vector<std::vector<ScorePair>> matchResults(N, std::vector<ScorePair>(N));

        // Round-robin: Every strategy plays against every other strategy
        for (size_t i = 0; i < strategies.size(); ++i) {
            for (size_t j = i; j < strategies.size(); ++j) {
                const auto& p1 = strategies[i];
                const StrategyPtr* p2_ptr;
				// when i==j, play with a clone of itself, to avoid state interference
                std::unique_ptr<Strategy> p2_clone;

                if (i == j)
                {
                    p2_clone = p1->clone();
                    // Important: Set a new random seed for the clone to ensure different random number sequences
                    p2_clone->setSeed(std::random_device{}());
                    p2_ptr = &p2_clone;
                }
                else
                {
					p2_ptr = &strategies[j];
                }
                const auto& p2 = *p2_ptr;
                std::vector<double> p1_scores;
                std::vector<double> p2_scores;

                for (int r = 0; r < repeats; ++r) {
                    // to clean flag state
                    p1.get()->reset();
                    p2.get()->reset();

                    ScorePair scores = runGame(p1, p2, rounds);
                    p1_scores.push_back(scores.first);
                    p2_scores.push_back(scores.second);
                    
                    // Fix: When a strategy plays itself (i==j), only add score once
                    if (i == j) {
                        // Same strategy playing itself, both scores are the same, only add once
                        allScores[p1->getName()].push_back(scores.first);
                    } else {
                        // Different strategies playing, add each score separately
                        allScores[p1->getName()].push_back(scores.first);
                        allScores[p2->getName()].push_back(scores.second);
                    }
                }

                // Calculate average for match table
                double avg_score1 = 0.0, avg_score2 = 0.0;
                for (double s : p1_scores) avg_score1 += s;
                for (double s : p2_scores) avg_score2 += s;
                avg_score1 /= repeats;
                avg_score2 /= repeats;

                matchResults[i][j] = { avg_score1, avg_score2 };
                if (i != j) {
                    matchResults[j][i] = { avg_score2, avg_score1 };
                }
            }
        }

        // Calculate overall statistics for each strategy (including confidence intervals)
        std::map<std::string, ScoreStats> stats;
        for (const auto& [name, scores] : allScores) {
            stats[name] = calculateStats(scores);
        }
        
        return { stats, matchResults };
    }


    // Noise Sweep: Run tournaments at different noise levels
    std::map<double, std::map<std::string, ScoreStats>> runNoiseSweep(
        std::vector<StrategyPtr>& strategies,
        int rounds,
        int repeats,
        const std::vector<double>& noise_levels) const {

        std::map<double, std::map<std::string, ScoreStats>> results;

        std::cout << "\n=================================================\n";
        std::cout << "       Noise Sweep Experiment\n";
        std::cout << "=================================================\n\n";

        for (double epsilon : noise_levels) {
            std::cout << "\n--- Testing noise level  = " << std::fixed << std::setprecision(2)
                << epsilon << " ---\n";
			Strategy::setNoise(epsilon); // Set static noise level
            // Run the tournament
            auto [tournamentResults, matchResults] = runTournament(strategies, rounds, repeats);
            results[epsilon] = tournamentResults;

            // Print results for this noise level
            std::cout << "\nAverage scores at noise   = " << epsilon << " (with 95% CI):\n";
            std::vector<std::pair<std::string, ScoreStats>> sorted_results(
                tournamentResults.begin(), tournamentResults.end());
            std::sort(sorted_results.begin(), sorted_results.end(),
                [](const auto& a, const auto& b) { return a.second.mean > b.second.mean; });

            for (const auto& [name, stats] : sorted_results) {
                std::cout << "  " << std::setw(15) << std::left << name << ": "
                    << std::fixed << std::setprecision(2) << stats.mean 
                    << "  [" << stats.ci_lower << ", " << stats.ci_upper << "]\n";
            }
        }

        return results;
    }
};

#endif // SIMULATOR_H