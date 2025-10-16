#ifndef SIMULATOR_H
#define SIMULATOR_H
#include <sstream>
#include "Strategy.h"
#include "PayoffMatrix.h"
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

// Template type aliases
template<typename ScoreType = double>
using ScorePair = std::pair<ScoreType, ScoreType>;

using StrategyPtr = std::unique_ptr<Strategy>;

inline std::string moveToString(Move m) {
    return m == Move::Cooperate ? "C (Cooperate)" : "D (Defect)";
}

// Template structure to hold statistical information about scores
template<typename ScoreType = double>
struct ScoreStats {
    ScoreType mean;
    ScoreType stdev;
    ScoreType ci_lower;
    ScoreType ci_upper;
    int n_samples;

    ScoreStats() : mean(0), stdev(0), ci_lower(0), ci_upper(0), n_samples(0) {}
    
    ScoreStats(ScoreType m, ScoreType sd, ScoreType ci_low, ScoreType ci_high, int n)
        : mean(m), stdev(sd), ci_lower(ci_low), ci_upper(ci_high), n_samples(n) {}
};

/**
 * @brief Template class for running Prisoner's Dilemma simulations
 * @tparam ScoreType The type used for scores (default: double)
 */
template<typename ScoreType = double>
class Simulator {
private:
    PayoffMatrix<ScoreType> payoff_matrix_;
    double noise_level_;  // Current noise level

    ScoreType getScore(Move m1, Move m2) const {
        return payoff_matrix_.getPayoff(m1, m2);
    }

public:
    // Constructor using PayoffMatrix (preferred)
    explicit Simulator(const PayoffMatrix<ScoreType>& matrix, double noise = 0.0) 
        : payoff_matrix_(matrix), noise_level_(noise) {}
    
    // Constructor from vector (backward compatibility)
    explicit Simulator(const std::vector<ScoreType>& config, double noise = 0.0) 
        : payoff_matrix_(config), noise_level_(noise) {}

    // Set noise level
    void setNoise(double epsilon) { 
        noise_level_ = epsilon; 
    }

    double getNoise() const { 
        return noise_level_; 
    }
    
    // Get the payoff matrix
    const PayoffMatrix<ScoreType>& getPayoffMatrix() const {
        return payoff_matrix_;
    }

    // Run a single match, considering noise
    ScorePair<ScoreType> runGame(const StrategyPtr& p1, const StrategyPtr& p2, int rounds) const {
        History history1;// player1's perspective: {my move, opponent's move}
        History history2;
        ScoreType score1 = ScoreType(0);
        ScoreType score2 = ScoreType(0);
        
        for (int i = 1; i <= rounds; ++i) {
            // Use decideWithNoise method to get decision with noise
            Move move1 = p1->decideWithNoise(history1);
            Move move2 = p2->decideWithNoise(history2);
            
            ScoreType round_score1 = getScore(move1, move2);
            ScoreType round_score2 = getScore(move2, move1);
            score1 += round_score1;
            score2 += round_score2;
            // update history, from each player's perspective
            history1.push_back({ move1, move2 });  // player1: I play move1, opponent plays move2
            history2.push_back({ move2, move1 });  // player2: I play move2, opponent plays move1
        }

        // SCB: If complexity cost is enabled, deduct it from final score
        if (Strategy::isSCBEnabled()) {
            ScoreType cost1 = ScoreType(p1->getComplexity() * Strategy::getSCBCostFactor() * rounds);
            ScoreType cost2 = ScoreType(p2->getComplexity() * Strategy::getSCBCostFactor() * rounds);
            score1 -= cost1;
            score2 -= cost2;
        }

        return { score1, score2 };
    }
    
    // Calculate mean and standard deviation from a vector of scores
    inline ScoreStats<ScoreType> calculateStats(const std::vector<ScoreType>& scores) const {
        ScoreStats<ScoreType> stats;
        stats.n_samples = static_cast<int>(scores.size());
        if (scores.empty()) return stats;

        ScoreType sum = std::accumulate(scores.begin(), scores.end(), ScoreType(0));
        stats.mean = sum / stats.n_samples;

        if (stats.n_samples > 1) {
            ScoreType variance = ScoreType(0);
            for (const ScoreType& s : scores) {
                ScoreType diff = s - stats.mean;
                variance += diff * diff;
            }
            variance /= (stats.n_samples - 1);
            stats.stdev = std::sqrt(variance);

            ScoreType margin = ScoreType(1.96) * (stats.stdev / std::sqrt(ScoreType(stats.n_samples)));
            stats.ci_lower = stats.mean - margin;
            stats.ci_upper = stats.mean + margin;
        }
        else {
            stats.stdev = ScoreType(0);
            stats.ci_lower = stats.ci_upper = stats.mean;
        }

        return stats;
    }
    // Standard tournament with confidence intervals
    // Returns a pair: first is strategy statistics results, second is match matrix (for printing)
    std::pair<std::map<std::string, ScoreStats<ScoreType>>, std::vector<std::vector<ScorePair<ScoreType>>>> 
    runTournament(const std::vector<StrategyPtr>& strategies, int rounds, int repeats) const {
		std::map<std::string, std::vector<ScoreType>> allScores; // collect all scores for each strategy
        // Initialize
        for (const auto& s : strategies) {
            allScores[s->getName()] = std::vector<ScoreType>();
        }

        // Store detailed match results for table display
        int N = static_cast<int>(strategies.size());
        std::vector<std::vector<ScorePair<ScoreType>>> matchResults(N, std::vector<ScorePair<ScoreType>>(N));

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
                std::vector<ScoreType> p1_scores;
                std::vector<ScoreType> p2_scores;

                for (int r = 0; r < repeats; ++r) {
                    // to clean flag state
                    p1.get()->reset();
                    p2.get()->reset();

                    ScorePair<ScoreType> scores = runGame(p1, p2, rounds);
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
                ScoreType avg_score1 = ScoreType(0), avg_score2 = ScoreType(0);
                for (const ScoreType& s : p1_scores) avg_score1 += s;
                for (const ScoreType& s : p2_scores) avg_score2 += s;
                avg_score1 /= repeats;
                avg_score2 /= repeats;

                matchResults[i][j] = { avg_score1, avg_score2 };
                if (i != j) {
                    matchResults[j][i] = { avg_score2, avg_score1 };
                }
            }
        }

        // Calculate overall statistics for each strategy (including confidence intervals)
        std::map<std::string, ScoreStats<ScoreType>> stats;
        for (const auto& [name, scores] : allScores) {
            stats[name] = calculateStats(scores);
        }
        
        return { stats, matchResults };
    }


    // Noise Sweep: Run tournaments at different noise levels
    std::map<double, std::map<std::string, ScoreStats<ScoreType>>> runNoiseSweep(
        std::vector<StrategyPtr>& strategies,
        int rounds,
        int repeats,
        const std::vector<double>& noise_levels) const {

        std::map<double, std::map<std::string, ScoreStats<ScoreType>>> results;

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
            std::vector<std::pair<std::string, ScoreStats<ScoreType>>> sorted_results(
                tournamentResults.begin(), tournamentResults.end());
            std::sort(sorted_results.begin(), sorted_results.end(),
                [](const auto& a, const auto& b) { return a.second.mean > b.second.mean; });

            for (const auto& [name, stats] : sorted_results) {
                std::cout << "  " << std::setw(15) << std::left << name << ": "
                    << std::fixed << std::setprecision(2) << static_cast<double>(stats.mean)
                    << "  [" << static_cast<double>(stats.ci_lower) << ", " 
                    << static_cast<double>(stats.ci_upper) << "]\n";
            }
        }

        return results;
    }
};

// Type aliases for backward compatibility and common usage
using DefaultSimulator = Simulator<double>;
using IntSimulator = Simulator<int>;
using DoubleScoreStats = ScoreStats<double>;
using IntScoreStats = ScoreStats<int>;

#endif // SIMULATOR_H