#ifndef RESULTSPRINTER_H
#define RESULTSPRINTER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Config.h"
#include "Strategy.h"
#include "Simulator.h"

/**
 * @class ResultsPrinter
 * @brief Class responsible for all output and printing functions
 * 
 * This class centralizes all printing functions from SimulatorRunner,
 * making the code more modular and maintainable.
 */
class ResultsPrinter {
public:
    explicit ResultsPrinter(const Config& config);

    // ==================== Configuration and Matrix Printing ====================
    
    /// Print simulator configuration information
    void printConfiguration(const std::vector<std::unique_ptr<Strategy>>& strategies) const;
    
    /// Print payoff matrix
    void printPayoffMatrix() const;

    // ==================== Tournament Results Printing ====================
    
    /// Print tournament results table
    void printTournamentResults(const std::map<std::string, DoubleScoreStats>& results) const;
    
    /// Print match results matrix (moved from Simulator)
    void printMatchTable(
        const std::vector<std::unique_ptr<Strategy>>& strategies,
        const std::vector<std::vector<std::pair<double, double>>>& matchResults) const;
    
    // ==================== Noise Analysis Printing ====================
    
    /// Print noise sweep results table (moved from Simulator)
    void printNoiseSweepTable(
        const std::map<double, std::map<std::string, DoubleScoreStats>>& results) const;
    
    /// Print noise analysis table
    void printNoiseAnalysisTable(
        const std::map<double, std::map<std::string, DoubleScoreStats>>& noise_results) const;
    
    /// Export noise analysis to CSV file
    void exportNoiseAnalysisToCSV(
        const std::map<double, std::map<std::string, DoubleScoreStats>>& noise_results,
        const std::string& filename) const;

    // ==================== Exploiter Mode Printing ====================
    
    /// Print exploiter match table
    void printExploiterMatchTable(
        const std::string& exploiter_name,
        const std::map<std::string, std::pair<double, double>>& matchAverages) const;
    
    /// Show exploiter vs individual opponent detailed match results (moved from Simulator)
    void showExploiterVsOpponent(
        const std::string& exploiter_name,
        const std::string& victim_name,
        const DoubleScoreStats& exploiter_stats,
        const DoubleScoreStats& victim_stats,
        int repeats,
        int rounds) const;
    
    /// Analyze exploiter strategy performance in mixed population (moved from Simulator)
    void analyzeMixedPopulation(
        const std::map<std::string, DoubleScoreStats>& results,
        const std::string& exploiter_name) const;
    
    /// Print exploiter noise comparison results (Q3 enhancement)
    void printExploiterNoiseComparison(
        const std::string& exploiter_name,
        const std::map<double, std::map<std::string, std::pair<DoubleScoreStats, DoubleScoreStats>>>& results,
        int repeats) const;

    // ==================== Evolution Simulation Printing ====================
    
    /// Print evolution simulation header
    void printEvolutionHeader() const;
    /// Print complete evolution history table (after all generations are complete)
    void printEvolutionHistory(
        const std::vector<std::map<std::string, double>>& history,
        const std::vector<std::unique_ptr<Strategy>>& strategies,
        const std::string& label) const;
    
    /// Print ESS (Evolutionarily Stable Strategy) analysis
    void printESSAnalysis(
        const std::vector<std::map<std::string, double>>& history,
        const std::vector<std::unique_ptr<Strategy>>& strategies,
        const std::string& label) const;
    // ==================== SCB (Strategic Complexity Budget) Printing ====================
    
    /// Print strategy complexity information table
    void printComplexityTable(const std::vector<std::unique_ptr<Strategy>>& strategies) const;
    
    /// Print SCB comparison results (tournament results comparison with/without SCB)
    void printSCBComparison(
        const std::map<std::string, DoubleScoreStats>& results_without_scb,
        const std::map<std::string, DoubleScoreStats>& results_with_scb) const;
    

    // ==================== Utility Functions ====================
    
    /// Format double value to string (2 decimal places)
    static std::string formatDouble(double value);
    
    /// Format double value to string (specified precision)
    static std::string formatDouble(double value, int precision);

private:
    const Config& config_;
};

#endif // RESULTSPRINTER_H
