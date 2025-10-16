#ifndef PAYOFFMATRIX_H
#define PAYOFFMATRIX_H

#include <array>
#include <stdexcept>
#include <string>
#include "Strategy.h"

/**
 * @brief Template class for Prisoner's Dilemma payoff matrix
 * @tparam ScoreType The type used for scores (e.g., double, int, Rational)
 * 
 * Payoff structure:
 * - T (Temptation): Defect vs Cooperate
 * - R (Reward): Cooperate vs Cooperate  
 * - P (Punishment): Defect vs Defect
 * - S (Sucker): Cooperate vs Defect
 * 
 * Valid Prisoner's Dilemma requires: T > R > P > S and 2R > T + S
 */
template<typename ScoreType = double>
class PayoffMatrix {
private:
    std::array<ScoreType, 4> payoffs_;  // [T, R, P, S]
    
public:
    // Default constructor with classic payoffs
    PayoffMatrix() : payoffs_{ScoreType(5), ScoreType(3), ScoreType(1), ScoreType(0)} {
        validatePayoffs();
    }
    
    // Constructor with individual payoffs
    PayoffMatrix(ScoreType T, ScoreType R, ScoreType P, ScoreType S) 
        : payoffs_{T, R, P, S} {
        validatePayoffs();
    }
    
    // Constructor from array
    explicit PayoffMatrix(const std::array<ScoreType, 4>& payoffs) 
        : payoffs_(payoffs) {
        validatePayoffs();
    }
    
    // Constructor from vector (for backward compatibility)
    explicit PayoffMatrix(const std::vector<ScoreType>& payoffs) {
        if (payoffs.size() != 4) {
            throw std::invalid_argument("Payoff vector must have exactly 4 elements");
        }
        for (size_t i = 0; i < 4; ++i) {
            payoffs_[i] = payoffs[i];
        }
        validatePayoffs();
    }
    
    // Get payoff for a specific outcome
    ScoreType getPayoff(Move myMove, Move oppMove) const {
        if (myMove == Move::Defect && oppMove == Move::Cooperate) {
            return payoffs_[0];  // T (Temptation)
        }
        if (myMove == Move::Cooperate && oppMove == Move::Cooperate) {
            return payoffs_[1];  // R (Reward)
        }
        if (myMove == Move::Defect && oppMove == Move::Defect) {
            return payoffs_[2];  // P (Punishment)
        }
        if (myMove == Move::Cooperate && oppMove == Move::Defect) {
            return payoffs_[3];  // S (Sucker)
        }
        return ScoreType(0);
    }
    
    // Getters for individual payoffs
    ScoreType getTemptation() const { return payoffs_[0]; }
    ScoreType getReward() const { return payoffs_[1]; }
    ScoreType getPunishment() const { return payoffs_[2]; }
    ScoreType getSucker() const { return payoffs_[3]; }
    
    // Get all payoffs as array
    const std::array<ScoreType, 4>& getPayoffs() const { return payoffs_; }
    
    // Get payoffs as vector (for backward compatibility)
    std::vector<ScoreType> getPayoffsVector() const {
        return std::vector<ScoreType>(payoffs_.begin(), payoffs_.end());
    }
    
    // Validate that this is a valid Prisoner's Dilemma
    bool isValidPrisonersDilemma() const {
        const ScoreType& T = payoffs_[0];
        const ScoreType& R = payoffs_[1];
        const ScoreType& P = payoffs_[2];
        const ScoreType& S = payoffs_[3];
        
        // Check T > R > P > S
        bool order_valid = (T > R) && (R > P) && (P > S);
        
        // Check 2R > T + S (prevent always defecting or always cooperating)
        bool cooperative_viable = (ScoreType(2) * R > T + S);
        
        return order_valid && cooperative_viable;
    }
    
    // Validate payoffs (throws exception if invalid)
    void validatePayoffs() const {
        if (!isValidPrisonersDilemma()) {
            const ScoreType& T = payoffs_[0];
            const ScoreType& R = payoffs_[1];
            const ScoreType& P = payoffs_[2];
            const ScoreType& S = payoffs_[3];
            
            std::string msg = "Invalid Prisoner's Dilemma payoffs!\n";
            msg += "Current: T=" + std::to_string(static_cast<double>(T)) + 
                   ", R=" + std::to_string(static_cast<double>(R)) + 
                   ", P=" + std::to_string(static_cast<double>(P)) + 
                   ", S=" + std::to_string(static_cast<double>(S)) + "\n";
            msg += "Required: T > R > P > S and 2R > T + S";
            
            throw std::invalid_argument(msg);
        }
    }
    
    // Print payoff information
    std::string toString() const {
        const ScoreType& T = payoffs_[0];
        const ScoreType& R = payoffs_[1];
        const ScoreType& P = payoffs_[2];
        const ScoreType& S = payoffs_[3];
        
        std::string result = "Payoff Matrix:\n";
        result += "  T (Temptation)  = " + std::to_string(static_cast<double>(T)) + "\n";
        result += "  R (Reward)      = " + std::to_string(static_cast<double>(R)) + "\n";
        result += "  P (Punishment)  = " + std::to_string(static_cast<double>(P)) + "\n";
        result += "  S (Sucker)      = " + std::to_string(static_cast<double>(S)) + "\n";
        result += "  Valid PD: " + std::string(isValidPrisonersDilemma() ? "Yes" : "No");
        
        return result;
    }
};

// Type aliases for common usage
using DoublePayoffMatrix = PayoffMatrix<double>;
using IntPayoffMatrix = PayoffMatrix<int>;

#endif // PAYOFFMATRIX_H
