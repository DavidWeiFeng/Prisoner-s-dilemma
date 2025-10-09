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
};

// ÓÀÔ¶±³ÅÑ (ALLD)
class AllDefect : public Strategy {
public:
    Move decide(const History& history) const override {
		return Move::Defect;
    }
    std::string getName() const override { return "ALLD"; }
};

// TODO: TFT, GRIM, PAVLOV µÈ²ßÂÔÉùÃ÷

class TitForTat : public Strategy {
public:
    Move decide(const History& history) const override {
        if (history.empty()) {
			return Move::Cooperate; // µÚÒ»ÂÖºÏ×÷
        }
		return history.back().second; // Ä£·Â¶ÔÊÖÉÏ´Î¶¯×÷
    }
	std::string getName() const override { return "TFT"; }
};

class GrimTrigger : public Strategy {
private:  mutable bool cooperateForever = true; // 标志位：是否继续合作
public:
    Move decide(const History& history) const override {
        //开始合作
        if (history.empty()) {
            return Move::Cooperate;
		}
        if (cooperateForever && history.back().second == Move::Defect)
        {
			cooperateForever = false; // 一旦对方背叛，永远背叛
        }
		return cooperateForever ? Move::Cooperate : Move::Defect;
    }
	std::string getName() const override { return "GRIM"; }
};

class PAVLOV: public Strategy {
public:
    Move decide(const History& history) const override {
        //开始合作
        if (history.empty()) {
            return Move::Cooperate;
        }
        if (history.back().first == history.back().second)
        {
			return history.back().first; // 如果上次双方选择相同，则继续选择相同
        }
        else
        {
			return history.back().first == Move::Cooperate ? Move::Defect : Move::Cooperate; // 否则切换选择
        }
    }
    std::string getName() const override { return "PAVLOV"; }
};

class RandomStrategy : public Strategy {
private:
    double p; // 合作概率
    mutable std::mt19937 gen; // 随机数生成器
    mutable std::uniform_real_distribution<double> dist; // 0~1均匀分布

public:
    RandomStrategy() : p(0.2), gen(std::random_device{}()), dist(0.0, 1.0) {
    };
    // 构造函数：prob为合作概率，seed可选，用于复现结果
    RandomStrategy(double prob, unsigned int seed = std::random_device{}())
        : p(prob), gen(seed), dist(0.0, 1.0) {
    }

    Move decide(const History& history) const override {
        double r = dist(gen); // 生成0~1随机数
        return (r < p) ? Move::Cooperate : Move::Defect;
    }

    std::string getName() const override {
        return "RND(""prob:"+ std::to_string(p) + ")";
    }
};

#endif // STRATEGIES_H

