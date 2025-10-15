#ifndef STRATEGY_H
#define STRATEGY_H

#include <vector>
#include <string>
#include <random>
enum class Move { Cooperate, Defect };

using History = std::vector<std::pair<Move, Move>>;


class Strategy {

inline static double noise=0.0; // Error rate
// SCB (Strategic Complexity Budget) 相关静态变量
inline static bool enable_scb = false;           // 是否启用战略复杂度预算
inline static double scb_cost_factor = 0.1;      // 每单位复杂度每轮的成本系数

private:
	mutable std::mt19937 gen; // Random number generator
    mutable std::uniform_real_distribution<double> dist{ 0.0, 1.0 };; // 0~1均匀分布

public:

    // 设置噪声参数
    static  void setNoise(double epsilon) { noise = epsilon; }
	void setSeed(unsigned int seed) { gen.seed(seed); }
    double getNoise() const { return noise; }
    
    // SCB: 设置复杂度预算参数
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

    // SCB: 返回策略的复杂度得分（纯虚函数，子类必须实现）
    virtual double getComplexity() const = 0;
    
    // SCB: 返回复杂度描述（可选，用于调试和报告）
    virtual std::string getComplexityReason() const {
        return "Default complexity";
    }
};


#endif