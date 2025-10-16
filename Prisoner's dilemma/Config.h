#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <string>


struct Config {
    // Tournament parameters
    int rounds = 50;
    int repeats = 5;
    double epsilon = 0;
    int seed = 42;
    std::vector<double> payoffs = { 5.0, 3.0, 1.0, 0.0 }; // T, R, P, S

    
    //std::vector<std::string> strategy_names = { "AllCooperate", "AllDefect","TitForTat","GrimTrigger","PAVLOV"};
    //std::vector<std::string> strategy_names = { "AllCooperate", "AllDefect","TitForTat","GrimTrigger","PAVLOV","ContriteTitForTat","PROBER", };
     //std::vector<std::string> strategy_names = { "AllDefect","AllCooperate","TitForTat","ContriteTitForTat","PAVLOV"};
    //std::vector<std::string> strategy_names = { "PROBER","AllCooperate","TitForTat" };
    std::vector<std::string> strategy_names = { "TitForTat","GrimTrigger","PAVLOV","ContriteTitForTat" };

    std::string format = "csv";
    std::string save_file;
    std::string load_file;
    
    // Q2: Noise sweep parameters
    bool noise_sweep = false;           // Whether to enable noise sweep mode
    std::vector<double> epsilon_values = {0.0, 0.05, 0.1, 0.15, 0.2}; // Noise level sweep values
    
    // Q3: Exploiter test parameters
    bool show_exploiter = false;       // Whether to show exploiter vs opponent detailed matches
    bool analyze_mixed = false;         // Whether to analyze exploiter performance in mixed population
    bool exploiter_noise_compare = false;  // Whether to compare exploiter behavior with and without noise
    
	// Q4: Evolution simulation parameters
    bool evolve = false;
    int generations = 50;
    
    // Q5: SCB (Strategic Complexity Budget) parameters
    bool enable_scb = false;           // Whether to enable Strategic Complexity Budget
    double scb_cost_factor = 0.1;      // Cost coefficient per complexity unit per round
    bool scb_compare = false;          // Whether to run SCB comparison mode (with/without SCB)
};

#endif // CONFIG_H
