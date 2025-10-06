#ifndef STRATEGIES_H
#define STRATEGIES_H

#include "Strategy.h"

/**
 * @brief 永远合作策略
 * 无论历史如何，总是选择 Cooperate。
 */
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

// TODO: TFT, GRIM, PAVLOV 等策略声明

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
#endif
#pragma once

