#ifndef STRATEGY_H
#define STRATEGY_H

#include <vector>
#include <string>

// ��Ҷ���
enum class Move { Cooperate, Defect };

// ��ʷ��¼ (pair: ���1����, ���2����)
using History = std::vector<std::pair<Move, Move>>;

/**
 * @brief ���Գ������
 */
class Strategy {
public:
    virtual ~Strategy() = default;

    // ������һ������
    virtual Move decide(const History& history) const = 0;

    // ��������
    virtual std::string getName() const = 0;
};

#endif
#pragma once
