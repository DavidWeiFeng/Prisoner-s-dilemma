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

class MemoryTwo : public Strategy {
public:
    Move decide(const History& history) const override {
        size_t round = history.size();

        // First two rounds: trust-building phase, default to cooperation
        if (round < 2) {
            return Move::Cooperate;
        }

        // Get the opponent's moves from the last two rounds
        Move oppLastMove = history[round - 1].second;       // Last round
        Move oppSecondLastMove = history[round - 2].second; // Second-to-last round

        // Decision rules: based on the opponent’s behavior in the past two rounds

        // Rule 1: Opponent cooperated in both rounds → reward cooperation
        if (oppLastMove == Move::Cooperate && oppSecondLastMove == Move::Cooperate) {
            return Move::Cooperate;
        }

        // Rule 2: Opponent defected in both rounds → recognize hostility and retaliate
        if (oppLastMove == Move::Defect && oppSecondLastMove == Move::Defect) {
            return Move::Defect;
        }

        // Rule 3: Mixed behavior (one cooperate, one defect) → give benefit of the doubt and cooperate
        // Rationale: could be a single mistake caused by noise, not worth immediate retaliation
        return Move::Cooperate;
    }

    std::string getName() const override { return "MEM2"; }

    std::unique_ptr<Strategy> clone() const override {
        return std::make_unique<MemoryTwo>(*this);
    }

    // SCB: Strategy complexity score
    double getComplexity() const override { return 2.5; }

    std::string getComplexityReason() const override {
        return "2-round memory + pattern recognition";
    }
};


class SoftGrudger : public Strategy {
private:
    // State machine: defines four working states of the strategy
    enum class State {
        COOPERATING,        // Cooperation mode
        PUNISHING,          // Punishment mode
        RECONCILING,        // Reconciliation mode
        PERMANENT_DEFECT    // Permanent defection mode
    };

    // State variables (mutable allows modification inside const functions)
    mutable State state = State::COOPERATING;
    mutable int punishCounter = 0;      // Counter for punishment rounds
    mutable int reconcileCounter = 0;   // Counter for reconciliation rounds

    // Configurable parameters
    static constexpr int PUNISH_ROUNDS = 4;     // Number of punishment rounds
    static constexpr int RECONCILE_ROUNDS = 2;  // Number of reconciliation rounds

public:
    Move decide(const History& history) const override {
        // First round: initialize state and cooperate
        if (history.empty()) {
            state = State::COOPERATING;
            return Move::Cooperate;
        }

        // Get opponent's move from the previous round
        Move oppLastMove = history.back().second;

        // Finite state machine: make decisions based on current state and opponent's move
        switch (state) {
            // ==================== State 1: Cooperation mode ====================
        case State::COOPERATING:
            // If the opponent defects, switch to punishment mode
            if (oppLastMove == Move::Defect) {
                state = State::PUNISHING;
                punishCounter = 1; // Start counting (current round counts as the first punishment)
                return Move::Defect; // Immediate retaliation
            }
            // Opponent cooperates → continue cooperating
            return Move::Cooperate;

            // ==================== State 2: Punishment mode ====================
        case State::PUNISHING:
            punishCounter++;

            // After punishment period ends → attempt reconciliation
            if (punishCounter >= PUNISH_ROUNDS) {
                state = State::RECONCILING;
                reconcileCounter = 0;
                return Move::Cooperate; // Offer an olive branch
            }

            // Continue punishing
            return Move::Defect;

            // ==================== State 3: Reconciliation mode ====================
        case State::RECONCILING:
            reconcileCounter++;

            // If the opponent defects again during reconciliation → no more forgiveness
            if (oppLastMove == Move::Defect) {
                state = State::PERMANENT_DEFECT;
                return Move::Defect;
            }

            // If reconciliation period ends and opponent cooperated consistently → restore trust
            if (reconcileCounter >= RECONCILE_ROUNDS) {
                state = State::COOPERATING;
            }

            // Continue testing opponent’s sincerity
            return Move::Cooperate;

            // ==================== State 4: Permanent defection mode ====================
        case State::PERMANENT_DEFECT:
            // Never forgive again
            return Move::Defect;
        }

        // Default return (should never reach here)
        return Move::Cooperate;
    }

    std::string getName() const override { return "SOFTG"; }

    // Reset function: called when a new game starts
    void reset() const override {
        state = State::COOPERATING;
        punishCounter = 0;
        reconcileCounter = 0;
    }

    std::unique_ptr<Strategy> clone() const override {
        return std::make_unique<SoftGrudger>(*this);
    }

    // SCB: Strategy complexity score
    double getComplexity() const override { return 4.0; }

    std::string getComplexityReason() const override {
        return "Multi-state FSM + round counters + forgiveness logic";
    }
};

#endif // STRATEGIES_H

