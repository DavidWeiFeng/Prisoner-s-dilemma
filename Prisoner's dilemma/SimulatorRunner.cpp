#include "SimulatorRunner.h"
#include "Strategies.h"
#include "CLI.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <fstream>

// Constructor initializes the simulator with payoffs from the configuration.
SimulatorRunner::SimulatorRunner(const Config& config)
    : config_(config), simulator_(config.payoffs, config.epsilon), printer_(config) {
}

// Main execution flow
void SimulatorRunner::run() {
    setupStrategies();
    printer_.printConfiguration(strategies_);
    printer_.printPayoffMatrix();
    
    // Q5: Print strategy complexity table
    if (config_.enable_scb || config_.scb_compare) {
        printer_.printComplexityTable(strategies_);
    }
    
    // Q5: SCB Comparison Mode
    if (config_.scb_compare) {
        runSCBComparison();
        return;  // Return after SCB comparison
    }
    
    // Q2: Noise sweep mode
    if (config_.noise_sweep) {
        runNoiseSweep();
        return;  // Return after noise sweep
    }
    
    // Q3: Exploiter noise comparison mode
    if (config_.show_exploiter && config_.exploiter_noise_compare) {
        runExploiterNoiseComparison();
        return;  // Return after running exploiter noise comparison
    }
	//Q4: Evolution mode
    else if (config_.evolve) {
        runEvolution();
    }
    else {
        runSimulation();
        printer_.printTournamentResults(results_);
        
        // Q3: If mixed population analysis is enabled
        if (config_.analyze_mixed) {
            runMixedPopulationAnalysis();
        }
    }
}

// Central location to create strategy instances from strategy names.
std::unique_ptr<Strategy> SimulatorRunner::createStrategy(const std::string& name) {
    if (name == "AllCooperate") return std::make_unique<AllCooperate>();
    if (name == "AllDefect") return std::make_unique<AllDefect>();
    if (name == "TitForTat") return std::make_unique<TitForTat>();
    if (name == "GrimTrigger") return std::make_unique<GrimTrigger>();
    if (name == "PAVLOV") return std::make_unique<PAVLOV>();
    if (name == "ContriteTitForTat") return std::make_unique<ContriteTitForTat>();
    if (name == "PROBER") return std::make_unique<PROBER>();
    
    // Parse RandomStrategy parameters, format: RandomStrategy<prob>
    // Example: RandomStrategy0.3 means prob=0.3
    if (name.rfind("RandomStrategy", 0) == 0) {
        if (name == "RandomStrategy") {
            // No parameters specified, use default value
            return std::make_unique<RandomStrategy>();
        }
        // Extract probability parameter
        std::string prob_str = name.substr(14); // "RandomStrategy" has length 14
        try {
            double prob = std::stod(prob_str);
            if (prob < 0.0 || prob > 1.0) {
                throw std::runtime_error("RandomStrategy probability must be between 0.0 and 1.0, got: " + prob_str);
            }
            return std::make_unique<RandomStrategy>(prob);
        }
        catch (const std::invalid_argument&) {
            throw std::runtime_error("Invalid probability format for RandomStrategy: " + prob_str);
        }
        catch (const std::out_of_range&) {
            throw std::runtime_error("Probability value out of range for RandomStrategy: " + prob_str);
        }
    }
    
    return nullptr;
}

// Set up the strategies to use in the tournament.
void SimulatorRunner::setupStrategies() {
    Strategy::setNoise(config_.epsilon);
    // SCB: Apply complexity budget configuration
    Strategy::enableSCB(config_.enable_scb);
    Strategy::setSCBCostFactor(config_.scb_cost_factor);
    
    for (const auto& name : config_.strategy_names) {
        auto strat = createStrategy(name);
        if (!strat) {
            throw std::runtime_error("Unknown strategy found: " + name);
        }
        strat->setSeed(config_.seed);
        strategies_.push_back(std::move(strat));
    }

    if (strategies_.size() < 2) {
        throw std::runtime_error("A tournament requires at least two strategies.");
    }
}

void SimulatorRunner::runSimulation() {
    std::cout << "\n--- Tournament Start ---\n";
    auto [stats, matchResults] = simulator_.runTournament(strategies_, config_.rounds, config_.repeats);
    results_ = stats;
    
    // Print match matrix
    printer_.printMatchTable(strategies_, matchResults);
}

void SimulatorRunner::runExploiter() {
    std::cout << "\n--- Exploiter Tournament Start ---\n";
    if (strategies_.empty()) return;

    const auto& exploiter = strategies_[0];
    std::string exploiter_name = exploiter->getName();
    std::cout << "Exploiter: " << exploiter_name << "\n";
    std::cout << "Victims: ";
    for (size_t i = 1; i < strategies_.size(); ++i) {
        std::cout << strategies_[i]->getName();
        if (i < strategies_.size() - 1) std::cout << ", ";
    }
    std::cout << "\n\n";

    // Store scores for all strategies
    std::map<std::string, std::vector<double>> allScores;
    allScores[exploiter_name] = std::vector<double>();

    std::map<std::string, std::pair<double, double>> matchAverages;

    // Initialize score vectors for each victim
    for (size_t i = 1; i < strategies_.size(); ++i) {
        allScores[strategies_[i]->getName()] = std::vector<double>();
    }

    for (size_t i = 1; i < strategies_.size(); ++i) {
        const auto& victim = strategies_[i];
        std::string victim_name = victim->getName();
        std::vector<double> exploiter_scores_this_match;
        std::vector<double> victim_scores_this_match;

        for (int r = 0; r < config_.repeats; ++r) {
            exploiter->reset();
            victim->reset();

            ScorePair scores = simulator_.runGame(exploiter, victim, config_.rounds);

            allScores[exploiter_name].push_back(scores.first);
            allScores[victim_name].push_back(scores.second);

            exploiter_scores_this_match.push_back(scores.first);
            victim_scores_this_match.push_back(scores.second);
        }
        
        double exploiter_avg = std::accumulate(exploiter_scores_this_match.begin(),
            exploiter_scores_this_match.end(), 0.0) / config_.repeats;
        double victim_avg = std::accumulate(victim_scores_this_match.begin(),
            victim_scores_this_match.end(), 0.0) / config_.repeats;
        matchAverages[victim_name] = { exploiter_avg, victim_avg };
    }
    
    // Use ResultsPrinter to print
    printer_.printExploiterMatchTable(exploiter_name, matchAverages);

    results_.clear();
    for (const auto& [name, scores] : allScores) {
        results_[name] = simulator_.calculateStats(scores);
    }
    std::cout << "\n--- All exploiter matches completed ---\n";
}

void SimulatorRunner::runEvolution() {
    printer_.printEvolutionHeader();

    auto history_noisefree = runSingleEvolution(0.0, "Noise-Free, epsilon=0.0");
    auto history_noisy = runSingleEvolution(config_.epsilon, "Noisy, epsilon=" + std::to_string(config_.epsilon));
}

std::vector<std::map<std::string, double>>
SimulatorRunner::runSingleEvolution(double noise, const std::string& label) {
    Strategy::setNoise(noise);

    std::map<std::string, double> populations;
    double initial_fraction = 1.0 / strategies_.size();
    for (const auto& s : strategies_) {
        populations[s->getName()] = initial_fraction;
    }
    
    std::vector<std::map<std::string, double>> history;
    for (int gen = 0; gen < config_.generations; gen++) {
        history.push_back(populations);
        
        if (gen == config_.generations - 1) break;

        auto fitness = calculateFitness(populations, config_.rounds, config_.repeats);
        updatePopulations(populations, fitness);
    }
    
    // Print history after all generations are complete
    printer_.printEvolutionHistory(history, strategies_, label);
    
    return history;
}

std::map<std::string, double> SimulatorRunner::calculateFitness(
    const std::map<std::string, double>& populations, int rounds, int repeats) {

    std::map<std::string, double> fitness;

    for (const auto& strat_i : strategies_) {
        std::string name_i = strat_i->getName();
        double pop_i = populations.at(name_i);

        if (pop_i < 1e-6) {
            fitness[name_i] = 0.0;
            continue;
        }

        double total_fitness = 0.0;

        for (const auto& strat_j : strategies_) {
            std::string name_j = strat_j->getName();
            double pop_j = populations.at(name_j);

            if (pop_j < 1e-6) continue;

            double avg_score = playMultipleGames(strat_i, strat_j, rounds, repeats);
            total_fitness += avg_score * pop_j;
        }

        fitness[name_i] = total_fitness;
    }

    return fitness;
}

double SimulatorRunner::playMultipleGames(
    const std::unique_ptr<Strategy>& strat_i,
    const std::unique_ptr<Strategy>& strat_j,
    int rounds, int repeats) {

    double total_score = 0.0;
    bool is_self_play = (strat_i->getName() == strat_j->getName());

    for (int r = 0; r < repeats; ++r) {
        strat_i->reset();
        strat_j->reset();

        ScorePair scores;
        if (is_self_play) {
            auto clone = strat_i->clone();
            scores = simulator_.runGame(strat_i, clone, rounds);
        }
        else {
            scores = simulator_.runGame(strat_i, strat_j, rounds);
        }
        total_score += scores.first;
    }

    return total_score / repeats;
}

void SimulatorRunner::updatePopulations(
    std::map<std::string, double>& populations,
    const std::map<std::string, double>& fitness) {

    double avg_fitness = 0.0;
    for (const auto& [name, pop] : populations) {
        avg_fitness += fitness.at(name) * pop;
    }

    if (avg_fitness < 1e-9) {
        std::cerr << "Warning: Average fitness is too low, skipping update.\n";
        return;
    }

    std::map<std::string, double> new_populations;
    for (const auto& [name, pop] : populations) {
        new_populations[name] = pop * (fitness.at(name) / avg_fitness);
    }

    populations = new_populations;

    double sum = 0.0;
    for (const auto& [name, pop] : populations) {
        sum += pop;
    }
    if (std::abs(sum - 1.0) > 1e-6) {
        std::cerr << "Warning: Population sum = " << sum << " (should be 1.0)\n";
    }
}

// Noise sweep: Run tournaments at different noise levels
void SimulatorRunner::runNoiseSweep() {
    std::cout << "\n=================================================\n";
    std::cout << "    Noise Sweep Analysis\n";
    std::cout << "=================================================\n\n";
    
    std::cout << "Testing noise levels: ";
    for (size_t i = 0; i < config_.epsilon_values.size(); ++i) {
        std::cout << config_.epsilon_values[i];
        if (i < config_.epsilon_values.size() - 1) std::cout << ", ";
    }
    std::cout << "\n\n";
    
    // Execute noise sweep
    auto noise_results = executeNoiseSweep(config_.epsilon_values);
    
    // Print and export results
    printer_.printNoiseAnalysisTable(noise_results);
    
    std::cout << "\n--- Noise sweep completed ---\n";
}

std::map<double, std::map<std::string, ScoreStats>> 
SimulatorRunner::executeNoiseSweep(const std::vector<double>& epsilon_values) {
    std::map<double, std::map<std::string, ScoreStats>> all_results;
    
    for (double epsilon : epsilon_values) {
        std::cout << "\n--- Running tournament with epsilon = " << epsilon << " ---\n";
        
        // Set current noise level
        Strategy::setNoise(epsilon);
        
        // Reset all strategy states
        for (auto& strategy : strategies_) {
            strategy->reset();
        }
        
        // Run tournament
        auto [stats, matchResults] = simulator_.runTournament(strategies_, config_.rounds, config_.repeats);
        
        // Print match matrix
        printer_.printMatchTable(strategies_, matchResults);
        
        // Store results
        all_results[epsilon] = stats;
        
		printer_.printTournamentResults(stats);
    }
    
    // Restore original noise setting
    Strategy::setNoise(config_.epsilon);
    
    return all_results;
}

// SCB: Run tournament with comparison (can be called manually for experiments)
void SimulatorRunner::runSCBComparison() {
    std::cout << "\n=================================================\n";
    std::cout << "    SCB Comparison Mode\n";
    std::cout << "=================================================\n\n";

    // First run: WITHOUT SCB
    std::cout << "\n--- Running Tournament WITHOUT SCB ---\n";
    Strategy::enableSCB(false);
    auto [results_without_scb, matchResults1] = simulator_.runTournament(strategies_, config_.rounds, config_.repeats);
    printer_.printMatchTable(strategies_, matchResults1);

    // Second run: WITH SCB
    std::cout << "\n--- Running Tournament WITH SCB ---\n";
    Strategy::enableSCB(true);
    Strategy::setSCBCostFactor(config_.scb_cost_factor);
    auto [results_with_scb, matchResults2] = simulator_.runTournament(strategies_, config_.rounds, config_.repeats);
    printer_.printMatchTable(strategies_, matchResults2);

    // Print comparison results
    printer_.printSCBComparison(results_without_scb, results_with_scb);

    // Restore original configuration
    Strategy::enableSCB(config_.enable_scb);
}

// Q3: Show exploiter vs opponent detailed matches
void SimulatorRunner::runShowExploiter() {
    if (strategies_.size() < 2) {
        std::cerr << "Error: Need at least 2 strategies (exploiter + victim(s))\n";
        return;
    }
    
    std::cout << "\n=================================================\n";
    std::cout << "    Exploiter Detailed Match Mode\n";
    std::cout << "=================================================\n\n";
    
    const auto& exploiter = strategies_[0];
    std::string exploiter_name = exploiter->getName();
    std::cout << "Exploiter: " << exploiter_name << "\n";
    std::cout << "Victims: ";
    for (size_t i = 1; i < strategies_.size(); ++i) {
        std::cout << strategies_[i]->getName();
        if (i < strategies_.size() - 1) std::cout << ", ";
    }
    std::cout << "\n";
    
    // Detailed match against each victim strategy
    for (size_t i = 1; i < strategies_.size(); ++i) {
        const auto& victim = strategies_[i];
        std::string victim_name = victim->getName();
        
        std::vector<double> exploiter_scores;
        std::vector<double> victim_scores;

        // Run multiple repeated experiments
        for (int r = 0; r < config_.repeats; ++r) {
            exploiter->reset();
            victim->reset();

            ScorePair scores = simulator_.runGame(exploiter, victim, config_.rounds);
            exploiter_scores.push_back(scores.first);
            victim_scores.push_back(scores.second);
        }

        // Calculate statistics
        ScoreStats exploiter_stats = simulator_.calculateStats(exploiter_scores);
        ScoreStats victim_stats = simulator_.calculateStats(victim_scores);
        
        // Use ResultsPrinter to print detailed results
        printer_.showExploiterVsOpponent(
            exploiter_name,
            victim_name,
            exploiter_stats,
            victim_stats,
            config_.repeats,
            config_.rounds
        );
    }
    
    std::cout << "\n--- All exploiter matches completed ---\n";
}

// Q3: Run exploiter noise comparison
void SimulatorRunner::runExploiterNoiseComparison() {
    if (strategies_.size() < 2) {
        std::cerr << "Error: Need at least 2 strategies (exploiter + victim(s))\n";
        return;
    }
    
    std::cout << "\n=================================================\n";
    std::cout << "    Exploiter Noise Comparison Mode\n";
    std::cout << "=================================================\n\n";
    
    const auto& exploiter = strategies_[0];
    std::string exploiter_name = exploiter->getName();
    
    std::cout << "Exploiter: " << exploiter_name << "\n";
    std::cout << "Testing with epsilon = 0.0 (no noise) and epsilon = " 
              << config_.epsilon << " (with noise)\n";
    std::cout << "Victims: ";
    for (size_t i = 1; i < strategies_.size(); ++i) {
        std::cout << strategies_[i]->getName();
        if (i < strategies_.size() - 1) std::cout << ", ";
    }
    std::cout << "\n";
    
    // Test two noise levels: 0.0 and config_.epsilon
    std::vector<double> noise_levels = {0.0, config_.epsilon};
    std::map<double, std::map<std::string, std::pair<ScoreStats, ScoreStats>>> results;
    
    for (double epsilon : noise_levels) {
        Strategy::setNoise(epsilon);
        std::cout << "\n--- Testing with epsilon = " << epsilon << " ---\n";
        
        for (size_t i = 1; i < strategies_.size(); ++i) {
            const auto& victim = strategies_[i];
            std::string victim_name = victim->getName();
            
            std::vector<double> exploiter_scores;
            std::vector<double> victim_scores;
            
            for (int r = 0; r < config_.repeats; ++r) {
                exploiter->reset();
                victim->reset();
                
                ScorePair scores = simulator_.runGame(exploiter, victim, config_.rounds);
                exploiter_scores.push_back(scores.first);
                victim_scores.push_back(scores.second);
            }
            
            ScoreStats exploiter_stats = simulator_.calculateStats(exploiter_scores);
            ScoreStats victim_stats = simulator_.calculateStats(victim_scores);
            
            results[epsilon][victim_name] = {exploiter_stats, victim_stats};
            
            // Print individual match results
            printer_.showExploiterVsOpponent(
                exploiter_name, victim_name,
                exploiter_stats, victim_stats,
                config_.repeats, config_.rounds
            );
        }
    }
    
    // Print noise comparison analysis
    printer_.printExploiterNoiseComparison(exploiter_name, results, config_.repeats);
    
    // Restore original noise setting
    Strategy::setNoise(config_.epsilon);
    
    std::cout << "\n--- Exploiter noise comparison completed ---\n";
}

// Q3: Analyze exploiter performance in mixed population
void SimulatorRunner::runMixedPopulationAnalysis() {
    // Detect whether there are exploiter strategies in the strategy list
    std::vector<std::string> exploiter_names = {"PROBER", "ALLD"};
    std::string found_exploiter;
    
    for (const auto& exploiter : exploiter_names) {
        if (results_.find(exploiter) != results_.end()) {
            found_exploiter = exploiter;
            break;
        }
    }
    
    if (found_exploiter.empty()) {
        std::cerr << "\nWarning: No exploiter strategy (PROBER or ALLD) found in tournament.\n";
        std::cerr << "         Mixed population analysis requires an exploiter strategy.\n";
        return;
    }
    
    // Call ResultsPrinter's analysis function
    printer_.analyzeMixedPopulation(results_, found_exploiter);
}


Config SimulatorRunner::parseArguments(int argc, char** argv) {
    CLI::App app{ "Iterated Prisoner's Dilemma Simulator" };
    Config config;

    app.add_option("--rounds", config.rounds, "Number of rounds per match.");
    app.add_option("--repeats", config.repeats, "Number of repetitions per match to compute the average score.");
    app.add_option("--epsilon", config.epsilon, "Probability of random action (error rate).");
    app.add_option("--seed", config.seed, "Random seed for reproducibility.");
    app.add_option("--payoffs", config.payoffs, "Payoff values [T, R, P, S].")->expected(4);
    app.add_option("--strategies,--strategy_names", config.strategy_names, "List of participating strategies.");
    app.add_flag("--evolve", config.evolve, "Enable evolutionary simulation mode.");
    app.add_option("--generations", config.generations, "Number of generations for the evolutionary simulation.");

    // Noise sweep parameters - Support both hyphen and underscore formats
    app.add_flag("--noise-sweep,--noise_sweep", config.noise_sweep, "Enable noise sweep analysis mode.");
    app.add_option("--epsilon-values,--epsilon_values", config.epsilon_values, "List of epsilon values for noise sweep.");

    // Q3: Exploiter test parameters
    app.add_flag("--show-exploiter,--show_exploiter", config.show_exploiter,
        "Show detailed exploiter vs opponent matches (first strategy is exploiter).");
    app.add_flag("--analyze-mixed,--analyze_mixed", config.analyze_mixed,
        "Analyze exploiter performance in mixed population (requires PROBER or ALLD in strategies).");
    app.add_flag("--exploiter-noise-compare,--exploiter_noise_compare", config.exploiter_noise_compare,
        "Compare exploiter behavior with and without noise (requires --show-exploiter).");

    // SCB: Add command-line parameters
    app.add_flag("--enable-scb,--enable_scb", config.enable_scb, "Enable Strategic Complexity Budget.");
    app.add_option("--scb-cost,--scb_cost", config.scb_cost_factor, "SCB cost factor per complexity unit per round.");

    // SCB Comparison mode
    app.add_flag("--scb-compare,--scb_compare", config.scb_compare, "Enable SCB comparison mode (runs tournament with and without SCB).");

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::CallForHelp& e) {
        std::cout << app.help() << std::endl;
        std::exit(0);   // Print help and exit
    }
    catch (const CLI::ParseError& e) {
        std::exit(app.exit(e));  // Print error and exit
    }


    return config;
}