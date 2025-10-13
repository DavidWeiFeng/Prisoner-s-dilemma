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

// 永远背叛 (ALLD)
class AllDefect : public Strategy {
public:
    Move decide(const History& history) const override {
		return Move::Defect;
    }
    std::string getName() const override { return "ALLD"; }
};

// TFT (Tit-For-Tat)
class TitForTat : public Strategy {
public:
    Move decide(const History& history) const override {
        if (history.empty()) {
			return Move::Cooperate; // 第一轮合作
        }
		return history.back().second; // 模仿对手上次动作
    }
	std::string getName() const override { return "TFT"; }
};

// GRIM (Grim Trigger)
class GrimTrigger : public Strategy {
private:  
    mutable bool cooperateForever = true; // 标志位：是否继续合作
public:
    Move decide(const History& history) const override {
        // 开始合作
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
    void reset() const override {
        cooperateForever = true;
    }
};

// PAVLOV (Win-Stay-Lose-Shift)
class PAVLOV: public Strategy {
public:
    Move decide(const History& history) const override {
        // 开始合作
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

// CTFT (Contrite Tit-For-Tat) - 悔悟的针锋相对
// 特点：能识别并修复因自己的噪声错误导致的背叛循环
class ContriteTitForTat : public Strategy {
private:
    mutable bool contrite = false; // 是否处于悔悟状态
public:
    Move decide(const History& history) const override {
        // 第一轮合作
        if (history.empty()) {
            contrite = false;
            return Move::Cooperate;
        }

        const auto& lastRound = history.back();
        Move myLastMove = lastRound.first;
        Move oppLastMove = lastRound.second;

        // 如果处于悔悟状态
        if (contrite) {
            // 如果对手上轮背叛（可能是对我的背叛的回应），我继续合作表示悔悟
            if (oppLastMove == Move::Defect) {
                contrite = false; // 结束悔悟状态
                return Move::Cooperate;
            }
            // 如果对手合作，说明我们恢复了合作
            contrite = false;
            return Move::Cooperate;
        }

        // 检查是否需要进入悔悟状态
        // 如果我上次背叛但对手合作，说明可能是我的噪声错误
        if (myLastMove == Move::Defect && oppLastMove == Move::Cooperate) {
            contrite = true;
            return Move::Cooperate;
        }

        // 正常的 TFT 行为：模仿对手
        return oppLastMove;
    }

    std::string getName() const override { return "CTFT"; }
    void reset() const override {
        contrite = false;
    }
};

// 随机策略
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
        return "RND(prob:" + std::to_string(p) + ")";
    }
};

// PROBER - 探测者策略（剥削型）
// 特点：通过试探找出可以剥削的对手
// 行为：
//   - 前四轮：C, D, C, C（合作，背叛，合作，合作）
//   - 如果对手在第2轮（我方背叛后）仍然合作，则认为可剥削，永远背叛
//   - 否则，采用 TFT 策略
class PROBER : public Strategy {
private:
    mutable bool exploiting = false; // 是否进入剥削模式
public:
    Move decide(const History& history) const override {
        size_t round = history.size();

        // 前四轮的探测序列：C, D, C, C
        if (round == 0) {
            return Move::Cooperate; // 第1轮：合作
        }
        if (round == 1) {
            return Move::Defect; // 第2轮：背叛（试探）
        }
        if (round == 2) {
            return Move::Cooperate; // 第3轮：合作
        }
        // 第4轮开始决定策略
        if (round == 3 && !exploiting) {
            if (history[1].second == Move::Cooperate) {
                    exploiting = true;
            }
			return Move::Cooperate; // 第4轮：合作
        }
        // 如果在剥削模式，永远背叛
        if (exploiting) {
            return Move::Defect;
        }
        // 否则使用 TFT 策略
        return history.back().second;
    }
    std::string getName() const override { return "PROBER"; }
    void reset() const  {
        exploiting = false;
    }
};

#endif // STRATEGIES_H

