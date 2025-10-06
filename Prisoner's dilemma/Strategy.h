#ifndef STRATEGY_H
#define STRATEGY_H

#include <vector>
#include <string>

// 玩家动作
enum class Move { Cooperate, Defect };

// 历史记录 (pair: 玩家1动作, 玩家2动作)
using History = std::vector<std::pair<Move, Move>>;

/**
 * @brief 策略抽象基类
 */
class Strategy {
public:
    virtual ~Strategy() = default;

    // 决定下一步动作
    virtual Move decide(const History& history) const = 0;

    // 策略名称
    virtual std::string getName() const = 0;
};

#endif
#pragma once
