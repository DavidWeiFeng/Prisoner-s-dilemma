#ifndef SIMULATORRUNNER_H
#define SIMULATORRUNNER_H

#include <vector>
#include <memory>
#include <map>
#include <string>
#include <algorithm>
#include "Config.h"
#include "Strategy.h"
#include "Simulator.h"
#include "ResultsPrinter.h"

/**
 * @class SimulatorRunner
 * @brief Coordinates the main application logic.
 *
 * This class is responsible for setting up strategies, running simulations, and coordinating output based on the provided configuration.
 * All printing functionality has been moved to the ResultsPrinter class.
 */
class SimulatorRunner {
public:
    // Initialize application state using configuration object.
    explicit SimulatorRunner(const Config& config);

    // Main entry point for executing the simulation.
    void run();

    // Static method for handling command-line argument parsing.
    static Config parseArguments(int argc, char** argv);

private:

    // Factory function to create strategy instances from strategy names.
    static std::unique_ptr<Strategy> createStrategy(const std::string& name);
    Config config_;
    std::vector<std::unique_ptr<Strategy>> strategies_;
    Simulator simulator_;
    std::map<std::string, ScoreStats> results_; // Store simulation results (including confidence intervals)
    ResultsPrinter printer_;


    // Initialize the strategy object vector based on the names in the configuration.
    void setupStrategies();

    // Execute the main tournament or evolution simulation.
    void runSimulation();
    void runExploiter();

    // New: Run evolution simulation
    void runEvolution();
    std::vector<std::map<std::string, double>> runSingleEvolution(double noise, const std::string& label);
    std::map<std::string, double> calculateFitness(const std::map<std::string, double>& populations, int rounds, int repeats);
    
    // New: Run noise sweep
    void runNoiseSweep();
    std::map<double, std::map<std::string, ScoreStats>> executeNoiseSweep(const std::vector<double>& epsilon_values);

    // Q3: Run exploiter detailed matches
    void runShowExploiter();
    
    // Q3: Run exploiter noise comparison
    void runExploiterNoiseComparison();
    
    // Q3: Run mixed population analysis
    void runMixedPopulationAnalysis();

    double playMultipleGames(
        const std::unique_ptr<Strategy>& strat_i,
        const std::unique_ptr<Strategy>& strat_j,
        int rounds, int repeats);

    void updatePopulations(
        std::map<std::string, double>& populations,
		const std::map<std::string, double>& fitness);

    // SCB: Run tournament with SCB comparison
    void runSCBComparison();
    void printExploiterMatchTable(
        const std::string& exploiter_name,
        const std::map<std::string, std::pair<double, double>>& matchAverages) const;

    void printExploiterResults(
        const std::string& exploiter_name,
		const std::map<std::string, std::pair<ScoreStats, ScoreStats>>& results) const;
};

#endif // SIMULATORRUNNER_H