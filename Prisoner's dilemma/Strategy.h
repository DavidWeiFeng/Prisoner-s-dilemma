#ifndef STRATEGY_H
#define STRATEGY_H

#include <vector>
#include <string>
#include <random>
enum class Move { Cooperate, Defect };

using History = std::vector<std::pair<Move, Move>>;


class Strategy {

inline static double noise=0.0; // Error rate
// SCB (Strategic Complexity Budget) related static variables
inline static bool enable_scb = false;           // Whether to enable Strategic Complexity Budget
inline static double scb_cost_factor = 0.1;      // Cost coefficient per complexity unit per round

private:
	mutable std::mt19937 gen; // Random number generator
    mutable std::uniform_real_distribution<double> dist{ 0.0, 1.0 };; // Uniform distribution from 0 to 1

public:

    // Set noise parameter
    static  void setNoise(double epsilon) { noise = epsilon; }
	void setSeed(unsigned int seed) { gen.seed(seed); }
    double getNoise() const { return noise; }
    
    // SCB: Set complexity budget parameters
    static void enableSCB(bool enable) { enable_scb = enable; }
    static void setSCBCostFactor(double factor) { scb_cost_factor = factor; }
    static bool isSCBEnabled() { return enable_scb; }
    static double getSCBCostFactor() { return scb_cost_factor; }
    
    virtual ~Strategy() = default;
    virtual Move decide(const History& history) const = 0;
    virtual std::string getName() const = 0;
    virtual std::unique_ptr<Strategy> clone() const = 0;

    Move applyNoise(Move move) const {
        if (noise == 0)
        {
            return move;
        }
        if (dist(gen) < noise) {
            return move == Move::Cooperate ? Move::Defect : Move::Cooperate;
        }
        return move;
    };

    virtual void reset() const {};

    Move decideWithNoise(const History& history) const {
        return applyNoise(decide(history));
    }

    // SCB: Return strategy's complexity score (pure virtual function, must be implemented by subclasses)
    virtual double getComplexity() const = 0;
    
    // SCB: Return complexity description (optional, for debugging and reporting)
    virtual std::string getComplexityReason() const {
        return "Default complexity";
    }
};


#endif