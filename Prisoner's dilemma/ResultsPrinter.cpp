#include "ResultsPrinter.h"
#include <tabulate/table.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>

ResultsPrinter::ResultsPrinter(const Config& config) : config_(config) {
}

// ==================== Utility Functions ====================

std::string ResultsPrinter::formatDouble(double value) {
    return formatDouble(value, 2);
}

std::string ResultsPrinter::formatDouble(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

// ==================== Configuration and Matrix Printing ====================

void ResultsPrinter::printConfiguration(const std::vector<std::unique_ptr<Strategy>>& strategies) const {
    std::cout << "\n=================================================\n";
    std::cout << "    Prisoner's Dilemma Simulator\n";
    std::cout << "=================================================\n\n";

    tabulate::Table table;

    // Simulation configuration
    table.add_row({ "Rounds per match", std::to_string(config_.rounds) });
    table.add_row({ "Repeats per match", std::to_string(config_.repeats) });
    table.add_row({ "Epsilon", std::to_string(config_.epsilon) });
    table.add_row({ "Random seed", std::to_string(config_.seed) });

    // Payoff values
    table.add_row({ "Payoffs (T,R,P,S)",
        std::to_string(config_.payoffs[0]) + ", " +
        std::to_string(config_.payoffs[1]) + ", " +
        std::to_string(config_.payoffs[2]) + ", " +
        std::to_string(config_.payoffs[3])
    });

    // Strategy list
    std::string strategy_list;
    for (const auto& s : strategies) {
        strategy_list += s->getName() + " ";
    }
    table.add_row({ "Participating strategies", strategy_list });

    // Evolution parameters
    if (config_.evolve) {
        table.add_row({ "Generations", std::to_string(config_.generations) });
    }

    // Format table
    table.format()
        .border_color(tabulate::Color::none)
        .font_align(tabulate::FontAlign::center);

    std::cout << table << std::endl;
}

void ResultsPrinter::printPayoffMatrix() const {
    double T = config_.payoffs[0];
    double R = config_.payoffs[1];
    double P = config_.payoffs[2];
    double S = config_.payoffs[3];

    std::cout << "\n--- Payoff Matrix ---\n";
    std::cout << "Based on the classic Prisoner's Dilemma parameters: T > R > P > S and 2R > T + S\n";
    std::cout << "\n";
    
    tabulate::Table table;

    // Set table header
    table.add_row({ "", "Opponent Cooperates (C)", "Opponent Defects (D)" });

    // First row: You Cooperate
    std::ostringstream oss1;
    oss1 << std::fixed << std::setprecision(2);
    oss1 << "R,R = " << R << "," << R;
    
    std::ostringstream oss2;
    oss2 << std::fixed << std::setprecision(2);
    oss2 << "S,T = " << S << "," << T;
    
    table.add_row({ "You Cooperate (C)", oss1.str(), oss2.str() });

    // Second row: You Defect
    std::ostringstream oss3;
    oss3 << std::fixed << std::setprecision(2);
    oss3 << "T,S = " << T << "," << S;
    
    std::ostringstream oss4;
    oss4 << std::fixed << std::setprecision(2);
    oss4 << "P,P = " << P << "," << P;
    
    table.add_row({ "You Defect (D)", oss3.str(), oss4.str() });

    // Format table
    table.format()
        .border_color(tabulate::Color::blue)
        .font_align(tabulate::FontAlign::center);

    std::cout << table << std::endl;
    
    std::cout << "\nWhere:\n";
    std::cout << "  T (Temptation) = " << T << "  - Temptation to defect against a cooperator\n";
    std::cout << "  R (Reward)     = " << R << "  - Reward for mutual cooperation\n";
    std::cout << "  P (Punishment) = " << P << "  - Punishment for mutual defection\n";
    std::cout << "  S (Sucker)     = " << S << "  - Payoff for being betrayed when cooperating\n";
    std::cout << "\n";
}

// ==================== Tournament Results Printing ====================

void ResultsPrinter::printTournamentResults(const std::map<std::string, DoubleScoreStats>& results) const {
std::cout << "\n=================================================\n";
std::cout << "--- Tournament Results (Average Score per Strategy) ---\n";
std::cout << "=================================================\n";

// Sort by average score
std::vector<std::pair<std::string, DoubleScoreStats>> sorted_results(results.begin(), results.end());
std::sort(sorted_results.begin(), sorted_results.end(),
    [](const auto& a, const auto& b) { return a.second.mean > b.second.mean; });

    std::cout << "Based on " << config_.repeats << " repeated experiments\n\n";

    tabulate::Table table;
    table.add_row({ "Rank", "Strategy", "Mean", "95% CI Lower", "95% CI Upper", "Std Dev" });

    int rank = 1;
    for (const auto& [name, stats] : sorted_results) {
        table.add_row({
            std::to_string(rank++),
            name,
            formatDouble(stats.mean),
            formatDouble(stats.ci_lower),
            formatDouble(stats.ci_upper),
            formatDouble(stats.stdev)
        });
    }
    
    // Apply styles
    table.format()
        .font_align(tabulate::FontAlign::center)
        .font_style({ tabulate::FontStyle::bold })
        .border_color(tabulate::Color::cyan);

    for (size_t i = 0; i < table.size(); ++i) {
        table[i].format()
            .font_style({ tabulate::FontStyle::bold })
            .font_align(tabulate::FontAlign::center);
    }

    std::cout << table << "\n";
}

void ResultsPrinter::printMatchTable(
    const std::vector<std::unique_ptr<Strategy>>& strategies,
    const std::vector<std::vector<std::pair<double, double>>>& matchResults) const {

    std::cout << "\n--- Match Result Matrix" << std::endl;
    std::cout << "Format: P1 score : P2 score\n\n";

    tabulate::Table table;

    // Table header
    std::vector<std::string> header = { "Strategy \\ Opponent" };
    for (const auto& s : strategies) {
        header.push_back(s->getName());
    }
    table.add_row({ header.begin(), header.end() }); 

    // Fill each row
    for (size_t i = 0; i < strategies.size(); ++i) {
        std::vector<std::string> row;
        row.push_back(strategies[i]->getName());  // Row header

        for (size_t j = 0; j < strategies.size(); ++j) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2);
            if (i == j) {
                // Diagonal: self vs self
                oss << matchResults[i][j].first;
            }
            else {
                // P1 vs P2
                oss << matchResults[i][j].first << " : " << matchResults[i][j].second;
            }
            row.push_back(oss.str());
        }
        table.add_row({row.begin(),row.end()});
    }

    // Format table
    table.format()
        .border_color(tabulate::Color::green)
        .font_align(tabulate::FontAlign::center);

    std::cout << table << std::endl;
}

// ==================== Noise Analysis Printing ====================

void ResultsPrinter::printNoiseSweepTable(const std::map<double, std::map<std::string, DoubleScoreStats>>& results) const {
    if (results.empty()) return;

    std::cout << "\n=================================================\n";
    std::cout << " Noise Sweep Summary\n";
    std::cout << "=================================================\n\n";

    // Get all strategy names
    std::vector<std::string> strategies;
    for (const auto& [name, stats] : results.begin()->second) {
        strategies.push_back(name);
    }

    // Print table header
    std::cout << std::setw(10) << "  (Noise)";
    for (const auto& name : strategies) {
        std::cout << std::setw(25) << name;
    }
    std::cout << "\n";
    std::cout << std::string(10 + strategies.size() * 25, '-') << "\n";

    // Print results for each noise level (mean ± CI)
    for (const auto& [epsilon, scores] : results) {
        std::cout << std::fixed << std::setprecision(2) << std::setw(10) << epsilon;
        for (const auto& name : strategies) {
            const auto& stats = scores.at(name);
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) 
                << stats.mean << " [" << stats.ci_lower << "," << stats.ci_upper << "]";
            std::cout << std::setw(25) << oss.str();
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void ResultsPrinter::printNoiseAnalysisTable(
const std::map<double, std::map<std::string, DoubleScoreStats>>& noise_results) const {
    
    std::cout << "\n=================================================\n";
    std::cout << "--- Noise Sweep Analysis Results ---\n";
    std::cout << "=================================================\n\n";
    
    // Collect all strategy names
    std::vector<std::string> strategy_names;
    if (!noise_results.empty()) {
        for (const auto& [strategy, _] : noise_results.begin()->second) {
            strategy_names.push_back(strategy);
        }
    }
    
    // Create table
    tabulate::Table table;
    
    // Table header
    std::vector<std::string> header = {"Epsilon (epsilon)"};
    for (const auto& name : strategy_names) {
        header.push_back(name);
    }
    table.add_row({header.begin(), header.end()});
    
    // Header style
    table[0].format()
        .font_style({tabulate::FontStyle::bold})
        .font_align(tabulate::FontAlign::center)
        .font_color(tabulate::Color::yellow);
    
    // Data rows
    for (const auto& [epsilon, results] : noise_results) {
        std::vector<std::string> row;
        row.push_back(formatDouble(epsilon, 2));
        
        for (const auto& name : strategy_names) {
            if (results.find(name) != results.end()) {
                row.push_back(formatDouble(results.at(name).mean));
            } else {
                row.push_back("N/A");
            }
        }
        
        table.add_row({row.begin(), row.end()});
    }
    
    // Table style
    table.format()
        .font_align(tabulate::FontAlign::center)
        .border_color(tabulate::Color::cyan);
    
    std::cout << table << "\n\n";
    
    // Print observations
    std::cout << "Observations:\n";
    std::cout << "  - Compare how each strategy's average payoff changes with noise level\n";
    std::cout << "  - Strategies with smaller drops are more noise-robust\n";
    std::cout << "  - Look for strategies that collapse (e.g., GRIM typically drops sharply)\n";
    std::cout << "  - CTFT and PAVLOV usually show better resilience to noise\n\n";
}

void ResultsPrinter::exportNoiseAnalysisToCSV(
const std::map<double, std::map<std::string, DoubleScoreStats>>& noise_results,
const std::string& filename) const {
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << " for writing.\n";
        return;
    }
    
    // CSV header
    file << "Epsilon,Strategy,Mean,StdDev,CI_Lower,CI_Upper\n";
    
    // Write data
    for (const auto& [epsilon, results] : noise_results) {
        for (const auto& [strategy, stats] : results) {
            file << formatDouble(epsilon, 2) << ","
                 << strategy << ","
                 << formatDouble(stats.mean) << ","
                 << formatDouble(stats.stdev) << ","
                 << formatDouble(stats.ci_lower) << ","
                 << formatDouble(stats.ci_upper) << "\n";
        }
    }
    
    file.close();
    std::cout << "Noise analysis exported to: " << filename << "\n";
}

// ==================== Exploiter Mode Printing ====================

void ResultsPrinter::printExploiterMatchTable(
    const std::string& exploiter_name,
    const std::map<std::string, std::pair<double, double>>& matchAverages) const {

    std::cout << "\n=================================================\n";
    std::cout << "--- Exploiter vs Victims: Average Scores ---\n";
    std::cout << "=================================================\n\n";

    tabulate::Table table;

    // Table header
    table.add_row({
        "Victim Strategy",
        exploiter_name + " Score",
        "Victim Score",
        "Score Difference"
    });

    // Header style
    table[0].format()
        .font_style({ tabulate::FontStyle::bold })
        .font_align(tabulate::FontAlign::center)
        .font_color(tabulate::Color::yellow);

    // Add data for each match
    double total_exploiter_score = 0.0;
    double total_victim_score = 0.0;

    for (const auto& [victim_name, scores] : matchAverages) {
        double exploiter_score = scores.first;
        double victim_score = scores.second;
        double difference = exploiter_score - victim_score;

        total_exploiter_score += exploiter_score;
        total_victim_score += victim_score;

        table.add_row({
            victim_name,
            formatDouble(exploiter_score),
            formatDouble(victim_score),
            formatDouble(difference)
        });

        // Set color based on score difference
        size_t row_idx = table.size() - 1;
        if (difference > 50) {
            table[row_idx][3].format().font_color(tabulate::Color::green);
        }
        else if (difference > 0) {
            table[row_idx][3].format().font_color(tabulate::Color::yellow);
        }
        else {
            table[row_idx][3].format().font_color(tabulate::Color::red);
        }
    }

    // Add totals row
    if (matchAverages.size() > 1) {
        double avg_exploiter = total_exploiter_score / matchAverages.size();
        double avg_victim = total_victim_score / matchAverages.size();
        double avg_difference = avg_exploiter - avg_victim;

        table.add_row({
            "Average",
            formatDouble(avg_exploiter),
            formatDouble(avg_victim),
            formatDouble(avg_difference)
        });

        // Bold totals row
        table[table.size() - 1].format()
            .font_style({ tabulate::FontStyle::bold })
            .font_color(tabulate::Color::cyan);
    }

    // Set table style
    table.format()
        .font_align(tabulate::FontAlign::center)
        .border_color(tabulate::Color::cyan)
        .border_top("═")
        .border_bottom("═")
        .border_left("║")
        .border_right("║");

    std::cout << table << "\n\n";

    // Print explanation
    std::cout << "Notes:\n";
    std::cout << "  - Each row shows the average score across " << config_.repeats << " matches.\n";
    std::cout << "  - Score Difference = " << exploiter_name << " Score - Victim Score\n";
    std::cout << "  - Positive difference (green/yellow) means " << exploiter_name << " is winning.\n";
    std::cout << "  - Negative difference (red) means the victim is resisting exploitation.\n\n";
}

void ResultsPrinter::showExploiterVsOpponent(
const std::string& exploiter_name,
const std::string& victim_name,
const DoubleScoreStats& exploiter_stats,
const DoubleScoreStats& victim_stats,
int repeats,
int rounds) const {

    std::cout << "\n=================================================\n";
    std::cout << "   Detailed Match: " << exploiter_name
        << " vs " << victim_name << "\n";
    std::cout << "=================================================\n\n";

    // Print results - Use tabulate library for consistent style
    std::cout << "Results after " << repeats << " matches of " << rounds << " rounds:\n\n";

    tabulate::Table table;
    table.add_row({ "Strategy", "Mean Score", "95% CI Lower", "95% CI Upper", "Std Dev" });

    table[0].format()
        .font_style({ tabulate::FontStyle::bold })
        .font_align(tabulate::FontAlign::center)
        .font_color(tabulate::Color::yellow);

    table.add_row({
        exploiter_name,
        formatDouble(exploiter_stats.mean),
        formatDouble(exploiter_stats.ci_lower),
        formatDouble(exploiter_stats.ci_upper),
        formatDouble(exploiter_stats.stdev)
        });

    table.add_row({
        victim_name,
        formatDouble(victim_stats.mean),
        formatDouble(victim_stats.ci_lower),
        formatDouble(victim_stats.ci_upper),
        formatDouble(victim_stats.stdev)
        });

    table.format()
        .font_align(tabulate::FontAlign::center)
        .border_color(tabulate::Color::cyan);

    std::cout << table << "\n";
}

void ResultsPrinter::analyzeMixedPopulation(
const std::map<std::string, DoubleScoreStats>& results,
const std::string& exploiter_name) const {
    
std::cout << "\n=================================================\n";
std::cout << "   Mixed Population Analysis: " << exploiter_name << "\n";
std::cout << "=================================================\n\n";

// Sort by score
std::vector<std::pair<std::string, DoubleScoreStats>> sorted_results(
    results.begin(), results.end());
std::sort(sorted_results.begin(), sorted_results.end(),
    [](const auto& a, const auto& b) { return a.second.mean > b.second.mean; });

    // Find exploiter's rank
    int exploiter_rank = 0;
    int total_strategies = sorted_results.size();
    
    for (int i = 0; i < total_strategies; ++i) {
        if (sorted_results[i].first == exploiter_name) {
            exploiter_rank = i + 1;
            break;
        }
    }

    // Print ranking table
    std::cout << "Performance Ranking:\n\n";
    std::cout << std::setw(5) << "Rank" 
              << std::setw(15) << "Strategy" 
              << std::setw(12) << "Avg Score"
              << std::setw(25) << "95% CI"
              << "  Notes\n";
    std::cout << std::string(77, '-') << "\n";

    int rank = 1;
    for (const auto& [name, stats] : sorted_results) {
        std::cout << std::setw(5) << rank 
                  << std::setw(15) << name
                  << std::setw(12) << std::fixed << std::setprecision(2) << stats.mean
                  << "  [" << std::setw(6) << stats.ci_lower 
                  << "," << std::setw(6) << stats.ci_upper << "]";
        
        if (name == exploiter_name) {
            std::cout << "  ← EXPLOITER";
        }
        std::cout << "\n";
        rank++;
    }

    // Analyze exploiter's performance
    std::cout << "\n--- Performance Analysis ---\n\n";
    
    auto exploiter_stats = results.at(exploiter_name);
    std::cout << exploiter_name << " finished in rank " << exploiter_rank 
              << " out of " << total_strategies << " strategies\n\n";

    if (exploiter_rank == 1) {
        std::cout << "✓ DOMINATES the population\n";
        std::cout << "  → High proportion of vulnerable strategies\n";
        std::cout << "  → Exploitation gains outweigh retaliation costs\n";
        std::cout << "  → This population is NOT stable (non-ESS)\n";
    } else if (exploiter_rank <= total_strategies / 2) {
        std::cout << "○ MODERATE performance\n";
        std::cout << "  → Successfully exploits some strategies\n";
        std::cout << "  → But punished by reciprocal strategies\n";
        std::cout << "  → Overall advantage is limited\n";
    } else {
        std::cout << "✗ POOR performance\n";
        std::cout << "  → Most strategies use retaliation\n";
        std::cout << "  → Trapped in mutual defection (P payoff)\n";
        std::cout << "  → Cannot compete with cooperative strategies\n";
        std::cout << "  → This is expected in diverse populations\n";
    }

    // Compare exploiter with top strategy
    if (exploiter_rank > 1) {
        const auto& top_strategy = sorted_results[0];
        double score_gap = top_strategy.second.mean - exploiter_stats.mean;
        
        std::cout << "\nScore gap with leader (" << top_strategy.first << "): "
                  << std::fixed << std::setprecision(2) << score_gap << " points\n";
        std::cout << "  → Reciprocal strategies maintain cooperation among themselves\n";
        std::cout << "  → This generates higher average scores than indiscriminate defection\n";
    }

    // Theoretical explanation
    std::cout << "\n--- Theoretical Insight ---\n\n";
    if (exploiter_name == "ALLD") {
        std::cout << "ALLD (Always Defect) in mixed populations:\n";
        std::cout << "  • Exploits unconditional cooperators (ALLC) → gains T payoff\n";
        std::cout << "  • But gets trapped in mutual defection with most others → receives P payoff\n";
        std::cout << "  • Since T > R > P > S, reciprocal strategies earning R outperform ALLD earning mostly P\n";
        std::cout << "  • Conclusion: Pure defection is NOT an Evolutionarily Stable Strategy (ESS)\n";
        std::cout << "                in populations with reciprocal strategies\n";
    } else if (exploiter_name == "PROBER") {
        std::cout << "PROBER in mixed populations:\n";
        std::cout << "  • Intelligently identifies exploitable targets (ALLC)\n";
        std::cout << "  • Switches to cooperation with defensive strategies (TFT, PAVLOV, CTFT)\n";
        std::cout << "  • More adaptive than ALLD, but success depends on population composition\n";
        std::cout << "  • Performance rank indicates the proportion of vulnerable vs. defensive strategies\n";
    }
    std::cout << "\n";
}

// ==================== Evolution Simulation Printing ====================

void ResultsPrinter::printEvolutionHeader() const {
    std::cout << "\n=================================================\n";
    std::cout << "    Evolutionary Tournament\n";
    std::cout << "=================================================\n\n";
}

void ResultsPrinter::printEvolutionHistory(
    const std::vector<std::map<std::string, double>>& history,
    const std::vector<std::unique_ptr<Strategy>>& strategies,
    const std::string& label) const {
    
    std::cout << "\n--- Evolution History (" << label << ") ---\n";
    
    tabulate::Table table;
    
    // Add table header
    std::vector<std::string> header = { "Generation" };
    for (const auto& s : strategies) {
        header.push_back(s->getName());
    }
    table.add_row({ header.begin(), header.end() });
    
    // Add data rows - Display every 4 generations
    for (size_t gen = 0; gen < history.size(); gen++) {
        if (gen % 4 == 0 || gen == history.size() - 1) {
            std::vector<std::string> row = { std::to_string(gen) };
            for (const auto& s : strategies) {
                row.push_back(formatDouble(history[gen].at(s->getName()), 3));
            }
            table.add_row({ row.begin(), row.end() });
        }
    }
    
    // Format table
    table.format()
        .font_align(tabulate::FontAlign::center)
        .border_color(tabulate::Color::cyan);
    
    // Bold and yellow header
    table[0].format()
        .font_style({ tabulate::FontStyle::bold });
    
    std::cout << table << "\n";
}

void ResultsPrinter::printESSAnalysis(
    const std::vector<std::map<std::string, double>>& history,
    const std::vector<std::unique_ptr<Strategy>>& strategies,
    const std::string& label) const {
    
    if (history.empty()) return;
    
    std::cout << "\n=================================================\n";
    std::cout << "   ESS (Evolutionarily Stable Strategy) Analysis\n";
    std::cout << "   " << label << "\n";
    std::cout << "=================================================\n\n";
    
    // Get final generation
    const auto& final_gen = history.back();
    
    // Find dominant strategy/strategies (population > 10%)
    std::vector<std::pair<std::string, double>> dominant_strategies;
    std::vector<std::pair<std::string, double>> surviving_strategies;
    std::vector<std::string> extinct_strategies;
    
    for (const auto& s : strategies) {
        std::string name = s->getName();
        double final_pop = final_gen.at(name);
        
        if (final_pop > 0.10) {
            dominant_strategies.push_back({name, final_pop});
        } else if (final_pop > 0.01) {
            surviving_strategies.push_back({name, final_pop});
        } else {
            extinct_strategies.push_back(name);
        }
    }
    
    // Sort by population descending
    std::sort(dominant_strategies.begin(), dominant_strategies.end(),
             [](const auto& a, const auto& b) { return a.second > b.second; });
    std::sort(surviving_strategies.begin(), surviving_strategies.end(),
             [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Print dominant strategies
    std::cout << "DOMINANT STRATEGIES (>10% population):\n";
    if (dominant_strategies.empty()) {
        std::cout << "  None - population is highly fragmented\n";
    } else {
        for (const auto& [name, pop] : dominant_strategies) {
            std::cout << "  • " << std::setw(20) << std::left << name 
                      << ": " << std::fixed << std::setprecision(1) 
                      << (pop * 100) << "%\n";
        }
    }
    
    std::cout << "\nSURVIVING STRATEGIES (1%-10% population):\n";
    if (surviving_strategies.empty()) {
        std::cout << "  None\n";
    } else {
        for (const auto& [name, pop] : surviving_strategies) {
            std::cout << "  • " << std::setw(20) << std::left << name 
                      << ": " << std::fixed << std::setprecision(1) 
                      << (pop * 100) << "%\n";
        }
    }
    
    std::cout << "\nEXTINCT/NEAR-EXTINCT STRATEGIES (<1% population):\n";
    if (extinct_strategies.empty()) {
        std::cout << "  None - all strategies survived!\n";
    } else {
        for (const auto& name : extinct_strategies) {
            std::cout << "  • " << name << "\n";
        }
    }
    
    // Analyze evolution trajectory
    std::cout << "\n--- EVOLUTIONARY TRAJECTORY ANALYSIS ---\n\n";
    
    // Track which strategies gained/lost population
    if (history.size() >= 2) {
        const auto& first_gen = history[0];
        std::vector<std::pair<std::string, double>> changes;
        
        for (const auto& s : strategies) {
            std::string name = s->getName();
            double change = final_gen.at(name) - first_gen.at(name);
            changes.push_back({name, change});
        }
        
        std::sort(changes.begin(), changes.end(),
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        std::cout << "Population changes from Generation 0 to " << (history.size() - 1) << ":\n";
        for (const auto& [name, change] : changes) {
            std::cout << "  " << std::setw(20) << std::left << name << ": ";
            if (change > 0) {
                std::cout << "+" << std::fixed << std::setprecision(1) << (change * 100) << "% (GAINING)";
            } else if (change < 0) {
                std::cout << std::fixed << std::setprecision(1) << (change * 100) << "% (DECLINING)";
            } else {
                std::cout << " 0.0% (STABLE)";
            }
            std::cout << "\n";
        }
    }
    
    // ESS theory discussion
    std::cout << "\n--- ESS THEORY INTERPRETATION ---\n\n";
    
    // Determine if we have an ESS
    if (dominant_strategies.size() == 1 && dominant_strategies[0].second > 0.90) {
        std::cout << "RESULT: Strong ESS detected - " << dominant_strategies[0].first << "\n\n";
        std::cout << "A single strategy dominates with >" << std::fixed << std::setprecision(0) 
                  << (dominant_strategies[0].second * 100) << "% of the population.\n";
        std::cout << "This indicates an Evolutionarily Stable Strategy (ESS) - a strategy that,\n";
        std::cout << "if adopted by most of the population, cannot be invaded by any alternative\n";
        std::cout << "strategy through natural selection.\n\n";
        
        // Strategy-specific insights
        std::string winner = dominant_strategies[0].first;
        if (winner == "TFT" || winner == "CTFT") {
            std::cout << "TFT/CTFT as ESS:\n";
            std::cout << "  • Reciprocal strategies form stable cooperative equilibria\n";
            std::cout << "  • They cooperate with themselves (R payoff) but retaliate against defectors\n";
            std::cout << "  • Defectors get trapped in mutual defection (P payoff) and cannot invade\n";
        } else if (winner == "PAVLOV") {
            std::cout << "PAVLOV as ESS:\n";
            std::cout << "  • Win-stay, lose-shift is highly adaptive\n";
            std::cout << "  • Can recover from occasional noise errors\n";
            std::cout << "  • Forms stable cooperation with similar strategies\n";
        } else if (winner == "ALLD") {
            std::cout << "ALLD (All Defect) as ESS:\n";
            std::cout << "  • In highly noisy environments, cooperation breaks down\n";
            std::cout << "  • Defection becomes the Nash equilibrium\n";
            std::cout << "  • This is a suboptimal but stable state (tragedy of the commons)\n";
        }
    } else if (dominant_strategies.size() > 1) {
        std::cout << "RESULT: Mixed ESS / Stable Polymorphism\n\n";
        std::cout << "Multiple strategies coexist in the population.\n";
        std::cout << "This suggests:\n";
        std::cout << "  • No single strategy can completely dominate\n";
        std::cout << "  • Different strategies exploit different ecological niches\n";
        std::cout << "  • A diverse population is more resilient to invasion\n\n";
        
        // Check for noise effects
        if (label.find("Noisy") != std::string::npos || label.find("epsilon") != std::string::npos) {
            std::cout << "NOISE IMPACT:\n";
            std::cout << "  • Noise disrupts pure cooperation strategies\n";
            std::cout << "  • Forgiving strategies (PAVLOV, CTFT) gain advantage\n";
            std::cout << "  • Strict strategies (GRIM) suffer from accidental betrayals\n";
        }
    } else {
        std::cout << "RESULT: No clear ESS - Population fragmented\n\n";
        std::cout << "All strategies maintain small populations.\n";
        std::cout << "This suggests:\n";
        std::cout << "  • The game parameters don't favor any particular strategy\n";
        std::cout << "  • The system may still be evolving towards equilibrium\n";
        std::cout << "  • Consider running more generations\n";
    }
    
    std::cout << "\n";
}

void ResultsPrinter::printSCBEvolutionProgress(
    int generation,
    const std::map<std::string, double>& populations,
    const std::vector<std::unique_ptr<Strategy>>& strategies,
    bool show_scb_costs) const {
    
    // Only print every 5 generations + first and last
    if (generation % 5 != 0 && generation != 0) {
        return;
    }
    
    std::cout << "\n--- Generation " << generation << " ---\n";
    
    tabulate::Table table;
    
    // Build header based on whether SCB is enabled
    std::vector<std::string> header = {"Strategy", "Population %"};
    if (show_scb_costs && Strategy::isSCBEnabled()) {
        header.push_back("Complexity");
        header.push_back("SCB Cost/Round");
    }
    table.add_row({header.begin(), header.end()});
    
    // Sort strategies by population (descending)
    std::vector<std::pair<std::string, double>> sorted_pops;
    for (const auto& s : strategies) {
        sorted_pops.push_back({s->getName(), populations.at(s->getName())});
    }
    std::sort(sorted_pops.begin(), sorted_pops.end(),
             [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Add data rows
    for (const auto& [name, pop] : sorted_pops) {
        double pop_percent = pop * 100.0;
        
        std::vector<std::string> row;
        row.push_back(name);
        row.push_back(formatDouble(pop_percent, 2) + "%");
        
        if (show_scb_costs && Strategy::isSCBEnabled()) {
            // Find strategy complexity
            double complexity = 0.0;
            for (const auto& s : strategies) {
                if (s->getName() == name) {
                    complexity = s->getComplexity();
                    break;
                }
            }
            
            double cost_per_round = complexity * Strategy::getSCBCostFactor();
            row.push_back(formatDouble(complexity, 1));
            row.push_back(formatDouble(cost_per_round, 3));
        }
        
        table.add_row({row.begin(), row.end()});
        
        // Color code based on population
        size_t row_idx = table.size() - 1;
        if (pop_percent > 20.0) {
            table[row_idx][0].format().font_color(tabulate::Color::green);
        } else if (pop_percent < 5.0) {
            table[row_idx][0].format().font_color(tabulate::Color::red);
        }
    }
    
    // Format table
    table[0].format()
        .font_style({tabulate::FontStyle::bold})
        .font_color(tabulate::Color::yellow);
    
    table.format()
        .font_align(tabulate::FontAlign::center)
        .border_color(tabulate::Color::cyan);
    
    std::cout << table << "\n";
    
    // Show top 3 strategies
    if (sorted_pops.size() >= 3) {
        std::cout << "Top 3: " << sorted_pops[0].first 
                  << " (" << formatDouble(sorted_pops[0].second * 100, 1) << "%), "
                  << sorted_pops[1].first 
                  << " (" << formatDouble(sorted_pops[1].second * 100, 1) << "%), "
                  << sorted_pops[2].first 
                  << " (" << formatDouble(sorted_pops[2].second * 100, 1) << "%)\n";
    }
}

// ==================== SCB (Strategic Complexity Budget) Printing ====================

void ResultsPrinter::printComplexityTable(const std::vector<std::unique_ptr<Strategy>>& strategies) const {
    std::cout << "\n=================================================\n";
    std::cout << "--- Strategy Complexity Table ---\n";
    std::cout << "=================================================\n\n";

    tabulate::Table table;

    // Table header
    table.add_row({ "Strategy", "Complexity Score", "Reason" });

    // Header style
    table[0].format()
        .font_style({ tabulate::FontStyle::bold })
        .font_align(tabulate::FontAlign::center)
        .font_color(tabulate::Color::yellow);

    // Add complexity information for each strategy
    for (const auto& s : strategies) {
        table.add_row({
            s->getName(),
            formatDouble(s->getComplexity(), 1),
            s->getComplexityReason()
        });
    }

    // Format table
    table.format()
        .font_align(tabulate::FontAlign::center)
        .border_color(tabulate::Color::cyan);

    std::cout << table << "\n\n";

    // Print explanation
    std::cout << "Complexity Score Interpretation:\n";
    std::cout << "  1.0 - Simplest strategies (no memory, fixed output)\n";
    std::cout << "  2.0 - Basic memory-based strategies (1-round memory)\n";
    std::cout << "  2.5 - Moderate complexity (state tracking + logic)\n";
    std::cout << "  3.5 - High complexity (multi-round memory, noise handling, probing)\n\n";
    
    if (Strategy::isSCBEnabled()) {
        std::cout << "SCB Status: ENABLED\n";
        std::cout << "Cost Factor: " << formatDouble(Strategy::getSCBCostFactor(), 2) 
                  << " per complexity unit per round\n";
        std::cout << "Formula: adjusted_score = raw_score - (complexity × cost_factor × rounds)\n\n";
    } else {
        std::cout << "SCB Status: DISABLED\n\n";
    }
}

void ResultsPrinter::printSCBComparison(
const std::map<std::string, DoubleScoreStats>& results_without_scb,
const std::map<std::string, DoubleScoreStats>& results_with_scb) const {

    std::cout << "\n=================================================\n";
    std::cout << "--- Tournament Results Comparison (With/Without SCB) ---\n";
    std::cout << "=================================================\n\n";

    // Collect all strategies and sort by score without SCB
    std::vector<std::string> strategy_names;
    for (const auto& [name, _] : results_without_scb) {
        strategy_names.push_back(name);
    }
    std::sort(strategy_names.begin(), strategy_names.end(),
        [&](const std::string& a, const std::string& b) {
            return results_without_scb.at(a).mean > results_without_scb.at(b).mean;
        });

    tabulate::Table table;

    // Table header
    table.add_row({
        "Strategy",
        "Without SCB",
        "Rank",
        "With SCB",
        "Rank",
        "Score Diff",
        "Rank Change"
    });

    // Header style
    table[0].format()
        .font_style({ tabulate::FontStyle::bold })
        .font_align(tabulate::FontAlign::center)
        .font_color(tabulate::Color::yellow);

    // Calculate rankings
    std::map<std::string, int> rank_without, rank_with;
    int r = 1;
    for (const auto& name : strategy_names) {
        rank_without[name] = r++;
    }

    // Re-sort by score with SCB to calculate rankings
    std::vector<std::string> sorted_with_scb = strategy_names;
    std::sort(sorted_with_scb.begin(), sorted_with_scb.end(),
        [&](const std::string& a, const std::string& b) {
            return results_with_scb.at(a).mean > results_with_scb.at(b).mean;
        });
    r = 1;
    for (const auto& name : sorted_with_scb) {
        rank_with[name] = r++;
    }

    // Fill table
    for (const auto& name : strategy_names) {
        double score_without = results_without_scb.at(name).mean;
        double score_with = results_with_scb.at(name).mean;
        double score_diff = score_with - score_without;
        int rank_change = rank_without[name] - rank_with[name];

        std::string rank_change_str;
        if (rank_change > 0) {
            rank_change_str = "↑" + std::to_string(rank_change);
        } else if (rank_change < 0) {
            rank_change_str = "↓" + std::to_string(-rank_change);
        } else {
            rank_change_str = "→";
        }

        table.add_row({
            name,
            formatDouble(score_without),
            std::to_string(rank_without[name]),
            formatDouble(score_with),
            std::to_string(rank_with[name]),
            formatDouble(score_diff),
            rank_change_str
        });

        // Set color based on rank change
        size_t row_idx = table.size() - 1;
        if (rank_change > 0) {
            table[row_idx][6].format().font_color(tabulate::Color::green);
        } else if (rank_change < 0) {
            table[row_idx][6].format().font_color(tabulate::Color::red);
        }
    }

    // Format table
    table.format()
        .font_align(tabulate::FontAlign::center)
        .border_color(tabulate::Color::cyan);

    std::cout << table << "\n\n";

    std::cout << "Notes:\n";
    std::cout << "  - Cost factor = " << formatDouble(Strategy::getSCBCostFactor(), 2) 
              << " per complexity unit per round\n";
    std::cout << "  - Rounds per match = " << config_.rounds << "\n";
    std::cout << "  - ↑ indicates rank improvement, ↓ indicates rank decline\n";
    std::cout << "  - Negative Score Diff means complexity cost reduced the score\n\n";
}
// ==================== Q3: Exploiter Noise Comparison ====================

void ResultsPrinter::printExploiterNoiseComparison(
const std::string& exploiter_name,
const std::map<double, std::map<std::string, std::pair<DoubleScoreStats, DoubleScoreStats>>>& results,
int repeats) const {
    
    std::cout << "\n=================================================\n";
    std::cout << "   Noise Impact on Exploitation\n";
    std::cout << "=================================================\n\n";
    
    // Get list of victims
    std::vector<std::string> victim_names;
    if (!results.empty()) {
        for (const auto& [victim, _] : results.begin()->second) {
            victim_names.push_back(victim);
        }
    }
    
    // Get list of epsilon values
    std::vector<double> epsilon_values;
    for (const auto& [epsilon, _] : results) {
        epsilon_values.push_back(epsilon);
    }
    
    tabulate::Table table;
    table.add_row({"Victim", "Epsilon", exploiter_name + " Score", 
                   "Victim Score", "Score Diff", "Change"});
    
    table[0].format()
        .font_style({tabulate::FontStyle::bold})
        .font_align(tabulate::FontAlign::center)
        .font_color(tabulate::Color::yellow);
    
    // Process each victim
    for (const auto& victim_name : victim_names) {
        double score_diff_no_noise = 0.0;
        
        // Get no-noise baseline
        if (results.count(0.0) > 0) {
            const auto& [exp_stats, vic_stats] = results.at(0.0).at(victim_name);
            score_diff_no_noise = exp_stats.mean - vic_stats.mean;
        }
        
        // Add rows for each epsilon value for this victim
        for (const auto& epsilon : epsilon_values) {
            const auto& [exp_stats, vic_stats] = results.at(epsilon).at(victim_name);
            double score_diff = exp_stats.mean - vic_stats.mean;
            double change = score_diff - score_diff_no_noise;
            
            std::string change_str;
            if (epsilon == 0.0) {
                change_str = "-";
            } else {
                change_str = (change > 0 ? "+" : "") + formatDouble(change);
            }
            
            table.add_row({
                victim_name,
                formatDouble(epsilon, 2),
                formatDouble(exp_stats.mean),
                formatDouble(vic_stats.mean),
                formatDouble(score_diff),
                change_str
            });
            
            // Color code the change column based on value
            size_t row_idx = table.size() - 1;
            if (epsilon != 0.0) {
                if (change < -10) {
                    table[row_idx][5].format().font_color(tabulate::Color::red);
                } else if (change < 0) {
                    table[row_idx][5].format().font_color(tabulate::Color::yellow);
                } else if (change > 0) {
                    table[row_idx][5].format().font_color(tabulate::Color::green);
                }
            }
        }
    }
    
    table.format()
        .font_align(tabulate::FontAlign::center)
        .border_color(tabulate::Color::cyan);
    
    std::cout << table << "\n\n";  
}
