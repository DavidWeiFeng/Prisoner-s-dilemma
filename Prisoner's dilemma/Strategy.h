#ifndef STRATEGY_H
#define STRATEGY_H

#include <vector>
#include <string>
#include <random>
enum class Move { Cooperate, Defect };

using History = std::vector<std::pair<Move, Move>>;


class Strategy {

double noise = 0.0; // Error rate
private:
	mutable std::mt19937 gen; // Random number generator
    mutable std::uniform_real_distribution<double> dist{ 0.0, 1.0 };; // 0~1均匀分布

public:

    // 设置噪声参数
    void setNoise(double epsilon) { noise = epsilon; }
	void setSeed(unsigned int seed) { gen.seed(seed); }
    double getNoise() const { return noise; }
    virtual ~Strategy() = default;

    virtual Move decide(const History& history) const = 0;

    virtual std::string getName() const = 0;

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

    Move decideWithNoise(const History& history) const {
        return applyNoise(decide(history));
    }
	virtual void reset() const {};
};

#endif