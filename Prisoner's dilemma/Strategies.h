#ifndef STRATEGIES_H
#define STRATEGIES_H

#include "Strategy.h"

/**
 * @brief ��Զ��������
 * ������ʷ��Σ�����ѡ�� Cooperate��
 */
class AllCooperate : public Strategy {
public:
	Move decide(const History& history) const override{
        return Move::Cooperate;
	}
    std::string getName() const override { return "ALLC"; }
};

// ��Զ���� (ALLD)
class AllDefect : public Strategy {
public:
    Move decide(const History& history) const override {
		return Move::Defect;
    }
    std::string getName() const override { return "ALLD"; }
};

// TODO: TFT, GRIM, PAVLOV �Ȳ�������

class TitForTat : public Strategy {
public:
    Move decide(const History& history) const override {
        if (history.empty()) {
			return Move::Cooperate; // ��һ�ֺ���
        }
		return history.back().second; // ģ�¶����ϴζ���
    }
	std::string getName() const override { return "TFT"; }
};
#endif
#pragma once

