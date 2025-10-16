#ifndef STRATEGIES_H
#define STRATEGIES_H

#include "Strategy.h"
#include <random>

class AllCooperate : public Strategy {
public:
	Move decide(const History& history) const override{
        return Move::Cooperate;
	}
    std::string getName() const override { return "ALLC"; }
    std::unique_ptr<Strategy> clone() const override {
        return std::make_unique<AllCooperate>(*this);
    }

    // SCB: Complexity score
    double getComplexity() const override { return 1.0; }
    std::string getComplexityReason() const override {
        return "No memory, fixed output";
    }
};

// Always Defect (ALLD)
class AllDefect : public Strategy {
public:
    Move decide(const History& history) const override {
		return Move::Defect;
    }
    std::string getName() const override { return "ALLD"; }
    std::unique_ptr<Strategy> clone() const override {
        return std::make_unique<AllDefect>(*this);
    }

    // SCB: Complexity score
    double getComplexity() const override { return 1.0; }
    std::string getComplexityReason() const override {
        return "No memory, fixed output";
    }
};

// TFT (Tit-For-Tat)
class TitForTat : public Strategy {
public:
    Move decide(const History& history) const override {
        if (history.empty()) {
			return Move::Cooperate; // Cooperate in the first round
        }
		return history.back().second; // Mimic opponent's last move
    }
	std::string getName() const override { return "TFT"; }

    std::unique_ptr<Strategy> clone() const override {
        return std::make_unique<TitForTat>(*this);
    }

    // SCB: Complexity score
    double getComplexity() const override { return 2.0; }
    std::string getComplexityReason() const override {
        return "1-round memory, simple mirroring";
    }
};

// GRIM (Grim Trigger)
class GrimTrigger : public Strategy {
private:  
    mutable bool cooperateForever = true; // Flag: whether to continue cooperating
public:
    Move decide(const History& history) const override {
        // Start by cooperating
        if (history.empty()) {
            return Move::Cooperate;
		}
        if (cooperateForever && history.back().second == Move::Defect)
        {
			cooperateForever = false; // Once opponent defects, defect forever
        }
		return cooperateForever ? Move::Cooperate : Move::Defect;
    }
	std::string getName() const override { return "GRIM"; }
    void reset() const override {
        cooperateForever = true;
    }
    std::unique_ptr<Strategy> clone() const override {
        return std::make_unique<GrimTrigger>(*this);
    }

    // SCB: Complexity score
    double getComplexity() const override { return 2.5; }
    std::string getComplexityReason() const override {
        return "Memory + permanent state switch";
    }
};

// PAVLOV (Win-Stay-Lose-Shift)
class PAVLOV: public Strategy {
public:
    Move decide(const History& history) const override {
        // Start by cooperating
        if (history.empty()) {
            return Move::Cooperate;
        }
        if (history.back().first == history.back().second)
        {
			return history.back().first; // If both chose the same last time, continue with the same choice
        }
        else
        {
			return history.back().first == Move::Cooperate ? Move::Defect : Move::Cooperate; // Otherwise, switch choice
        }
    }
    std::string getName() const override { return "PAVLOV"; }
    std::unique_ptr<Strategy> clone() const override {
        return std::make_unique<PAVLOV>(*this);
    }

    // SCB: Complexity score
    double getComplexity() const override { return 2.5; }
    std::string getComplexityReason() const override {
        return "Outcome memory + conditional logic";
    }
};

// CTFT (Contrite Tit-For-Tat) - Contrite Tit-For-Tat
// Feature: Can identify and repair defection loops caused by its own noise errors
class ContriteTitForTat : public Strategy {
private:
    mutable bool contrite = false; // Whether in contrite state
public:
    Move decide(const History& history) const override {
        // Cooperate in the first round
        if (history.empty()) {
            contrite = false;
            return Move::Cooperate;
        }

        const auto& lastRound = history.back();
        Move myLastMove = lastRound.first;
        Move oppLastMove = lastRound.second;

        // If in contrite state
        if (contrite) {
            // If opponent defected last round (possibly in response to my defection), I continue cooperating to show contrition
            if (oppLastMove == Move::Defect) {
                contrite = false; // End contrite state
                return Move::Cooperate;
            }
            // If opponent cooperated, it means we've restored cooperation
            contrite = false;
            return Move::Cooperate;
        }

        // Check if we need to enter contrite state
        // If I defected last time but opponent cooperated, it may be due to my noise error
        if (myLastMove == Move::Defect && oppLastMove == Move::Cooperate) {
            contrite = true;
            return Move::Cooperate;
        }

        // Normal TFT behavior: mimic opponent
        return oppLastMove;
    }

    std::string getName() const override { return "CTFT"; }

    void reset() const override {
        contrite = false;
    }
    std::unique_ptr<Strategy> clone() const override {
        return std::make_unique<ContriteTitForTat>(*this);
    }

    // SCB: Complexity score
    double getComplexity() const override { return 3.5; }
    std::string getComplexityReason() const override {
        return "Multi-round memory + noise detection";
    }
};

// Random Strategy
class RandomStrategy : public Strategy {
private:
    double      p; // Cooperation probability
    mutable std::mt19937 gen; // Random number generator
    mutable std::uniform_real_distribution<double> dist; // Uniform distribution from 0 to 1

public:
    RandomStrategy() : p(0.2), gen(std::random_device{}()), dist(0.0, 1.0) {
    };
    // Constructor: prob is cooperation probability, seed is optional for reproducibility
    RandomStrategy(double prob, unsigned int seed = std::random_device{}())
        : p(prob), gen(seed), dist(0.0, 1.0) {
    }

    Move decide(const History& history) const override {
        double r = dist(gen); // Generate random number from 0 to 1
        return (r < p) ? Move::Cooperate : Move::Defect;
    }

    std::string getName() const override {
        return "RND(prob:" + std::to_string(p) + ")";
    }

    std::unique_ptr<Strategy> clone() const override {
        // Create new random seed to ensure clone has different random number sequence
        return std::make_unique<RandomStrategy>(p, std::random_device{}());
    }

    // SCB: Complexity score
    double getComplexity() const override { return 1.5; }
    std::string getComplexityReason() const override {
        return "Random number generation";
    }
};

// PROBER - Prober strategy (exploiter type)
// Feature: Finds exploitable opponents through probing
// Behavior:
//   - First four rounds: C, D, C, C (cooperate, defect, cooperate, cooperate)
//   - If opponent still cooperates in round 2 (after our defection), consider it exploitable, always defect
//   - Otherwise, adopt TFT strategy
class PROBER : public Strategy {
private:
    mutable bool exploiting = false; // Whether in exploitation mode
public:
    Move decide(const History& history) const override {
        size_t round = history.size();

        // First four rounds probe sequence: C, D, C, C
        if (round == 0) {
            return Move::Cooperate; // Round 1: cooperate
        }
        if (round == 1) {
            return Move::Defect; // Round 2: defect (probe)
        }
        if (round == 2) {
            return Move::Cooperate; // Round 3: cooperate
        }
        // Starting from round 4, decide strategy
        if (round == 3 && !exploiting) {
            if (history[1].second == Move::Cooperate) {
                exploiting = true;
            }
            return Move::Cooperate; // Round 4: cooperate
        }
        // If in exploitation mode, always defect
        if (exploiting) {
            return Move::Defect;
        }
        // Otherwise use TFT strategy
        return history.back().second;
    }
    std::string getName() const override { return "PROBER"; }
    void reset() const {
        exploiting = false;
    }
    std::unique_ptr<Strategy> clone() const override {
        return std::make_unique<PROBER>(*this);
    }

    // SCB: Complexity score
    double getComplexity() const override { return 3.5; }
    std::string getComplexityReason() const override {
        return "Probe sequence + conditional branching";
    }
};


#endif // STRATEGIES_H

