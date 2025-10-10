#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <sstream>
#include "Strategy.h"
#include <iostream>
#include <iomanip>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

using ScorePair = std::pair<double, double>;
using StrategyPtr = std::unique_ptr<Strategy>;

inline std::string moveToString(Move m) {
    return m == Move::Cooperate ? "C (Cooperate)" : "D (Defect)";
}

class Simulator {
private:
    std::vector<double> payoff_config;
    double noise_level;  // 当前噪声水平

    double getScore(Move m1, Move m2) const {
        if (m1 == Move::Defect && m2 == Move::Cooperate) { return payoff_config[0]; }//T
        if (m1 == Move::Cooperate && m2 == Move::Cooperate) { return payoff_config[1]; }//R
        if (m1 == Move::Defect && m2 == Move::Defect) { return payoff_config[2]; }//P
        if (m1 == Move::Cooperate && m2 == Move::Defect) { return payoff_config[3]; }//S
        return 0.0;
    }

public:
    explicit Simulator(const std::vector<double>& config, double noise = 0.0) 
        : payoff_config(config), noise_level(noise) {}

    // 设置噪声水平
    void setNoise(double epsilon) { 
        noise_level = epsilon; 
    }

    double getNoise() const { 
        return noise_level; 
    }

    // 运行单场比赛，考虑噪声
    ScorePair runGame(const StrategyPtr& p1, const StrategyPtr& p2, int rounds) const {
        History history;
        double score1 = 0.0;
        double score2 = 0.0;
        
        for (int i = 1; i <= rounds; ++i) {
            // 使用 decideWithNoise 方法获取带噪声的决策
            Move move1 = p1->decideWithNoise(history);
            Move move2 = p2->decideWithNoise(history);
            
            double round_score1 = getScore(move1, move2);
            double round_score2 = getScore(move2, move1);
            score1 += round_score1;
            score2 += round_score2;
            history.push_back({ move1, move2 });
        }
        return { score1, score2 };
    }

    // 标准锦标赛
    std::map<std::string, double> runTournament(const std::vector<StrategyPtr>& strategies, 
                                                  int rounds, int repeats) const {
        std::map<std::string, double> totalScores;
        std::map<std::string, int> matchCounts;

        // 初始化
        for (const auto& s : strategies) {
            totalScores[s->getName()] = 0.0;
            matchCounts[s->getName()] = 0;
        }

        // 存储详细对战结果用于表格显示
        std::vector<std::vector<std::pair<double, double>>> matchResults(strategies.size(),
            std::vector<std::pair<double, double>>(strategies.size()));

        // 循环赛：每个策略两两对战
        for (size_t i = 0; i < strategies.size(); ++i) {
            for (size_t j = i; j < strategies.size(); ++j) {
                const auto& p1 = strategies[i];
                const auto& p2 = strategies[j];
                double cumulative_score1 = 0.0;
                double cumulative_score2 = 0.0;

                for (int r = 0; r < repeats; ++r) {
                    ScorePair scores = runGame(p1, p2, rounds);
                    cumulative_score1 += scores.first;
                    cumulative_score2 += scores.second;
                }

                double avg_score1 = cumulative_score1 / repeats;
                double avg_score2 = cumulative_score2 / repeats;

                matchResults[i][j] = { avg_score1, avg_score2 };
                if (i != j) {
                    matchResults[j][i] = { avg_score2, avg_score1 };
                }

                totalScores[p1->getName()] += cumulative_score1;
                totalScores[p2->getName()] += cumulative_score2;
                matchCounts[p1->getName()] += repeats;
                matchCounts[p2->getName()] += repeats;
            }
        }

        // 打印对战结果表格
        printMatchTable(strategies, matchResults);

        // 计算并返回全局平均分
        std::map<std::string, double> avg_scores;
        for (const auto& [name, total] : totalScores) {
            if (matchCounts.at(name) > 0) {
                avg_scores[name] = total / matchCounts.at(name);
            }
        }
        return avg_scores;
    }

    // 噪声扫描：在不同噪声水平下运行锦标赛
    std::map<double, std::map<std::string, double>> runNoiseSweep(
        std::vector<StrategyPtr>& strategies,
        int rounds,
        int repeats,
        const std::vector<double>& noise_levels) const {
        
        std::map<double, std::map<std::string, double>> results;

        std::cout << "\n=================================================\n";
        std::cout << "   噪声扫描实验 (Noise Sweep Experiment)\n";
        std::cout << "=================================================\n\n";

        for (double epsilon : noise_levels) {
            std::cout << "\n--- 测试噪声水平 ε = " << std::fixed << std::setprecision(2) 
                      << epsilon << " ---\n";

            // 为所有策略设置噪声
            for (auto& s : strategies) {
                s->setNoise(epsilon);
            }

            // 运行锦标赛
            std::map<std::string, double> tournamentResults = runTournament(strategies, rounds, repeats);
            results[epsilon] = tournamentResults;

            // 打印该噪声水平下的结果
            std::cout << "\n噪声 ε = " << epsilon << " 下的平均得分:\n";
            std::vector<std::pair<std::string, double>> sorted_results(
                tournamentResults.begin(), tournamentResults.end());
            std::sort(sorted_results.begin(), sorted_results.end(),
                [](const auto& a, const auto& b) { return a.second > b.second; });

            for (const auto& [name, score] : sorted_results) {
                std::cout << "  " << std::setw(15) << std::left << name << ": "
                    << std::fixed << std::setprecision(2) << score << "\n";
            }
        }

        return results;
    }

    // 打印噪声扫描结果表格
    static void printNoiseSweepTable(const std::map<double, std::map<std::string, double>>& results) {
        if (results.empty()) return;

        std::cout << "\n=================================================\n";
        std::cout << "   噪声扫描结果汇总表 (Noise Sweep Summary)\n";
        std::cout << "=================================================\n\n";

        // 获取所有策略名称
        std::vector<std::string> strategies;
        for (const auto& [name, score] : results.begin()->second) {
            strategies.push_back(name);
        }

        // 打印表头
        std::cout << std::setw(10) << "ε (Noise)";
        for (const auto& name : strategies) {
            std::cout << std::setw(15) << name;
        }
        std::cout << "\n";
        std::cout << std::string(10 + strategies.size() * 15, '-') << "\n";

        // 打印每个噪声水平的结果
        for (const auto& [epsilon, scores] : results) {
            std::cout << std::fixed << std::setprecision(2) << std::setw(10) << epsilon;
            for (const auto& name : strategies) {
                std::cout << std::setw(15) << std::fixed << std::setprecision(2) 
                          << scores.at(name);
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    // 分析噪声对策略的影响
    static void analyzeNoiseImpact(const std::map<double, std::map<std::string, double>>& results) {
        if (results.empty()) return;

        std::cout << "\n=================================================\n";
        std::cout << "   噪声影响分析 (Noise Impact Analysis)\n";
        std::cout << "=================================================\n\n";

        // 获取无噪声和最高噪声的结果
        auto baseline = results.begin()->second;  // ε = 0
        auto highest = results.rbegin()->second;  // 最高 ε

        std::cout << "性能下降分析 (从 ε=0 到 ε=" << std::fixed << std::setprecision(2) 
                  << results.rbegin()->first << "):\n\n";

        std::vector<std::pair<std::string, double>> performance_drops;
        for (const auto& [name, base_score] : baseline) {
            double drop_percent = ((base_score - highest.at(name)) / base_score) * 100.0;
            performance_drops.push_back({ name, drop_percent });
        }

        // 按性能下降排序
        std::sort(performance_drops.begin(), performance_drops.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        for (const auto& [name, drop] : performance_drops) {
            std::cout << std::setw(15) << std::left << name << ": "
                << std::fixed << std::setprecision(1) << drop << "% 下降";
            
            if (drop > 50) {
                std::cout << "  [严重崩溃]";
            } else if (drop > 30) {
                std::cout << "  [显著下降]";
            } else if (drop > 10) {
                std::cout << "  [中等影响]";
            } else {
                std::cout << "  [稳健]";
            }
            std::cout << "\n";
        }

        std::cout << "\n策略特性分析:\n\n";
        std::cout << "[严重崩溃的策略] (> 50%% 下降):\n";
        std::cout << "   - 通常是不宽容的策略 (如 GRIM)\n";
        std::cout << "   - 噪声导致永久性背叛循环\n";
        std::cout << "   - 无法从偶然错误中恢复\n\n";

        std::cout << "[中等影响的策略] (10-30%% 下降):\n";
        std::cout << "   - 部分宽容的策略 (如 TFT)\n";
        std::cout << "   - 能从错误恢复但需要时间\n";
        std::cout << "   - 可能陷入短期背叛循环\n\n";

        std::cout << "[稳健的策略] (< 10%% 下降):\n";
        std::cout << "   - 宽容且能纠错的策略 (如 CTFT, PAVLOV)\n";
        std::cout << "   - 能识别并修复噪声导致的错误\n";
        std::cout << "   - 快速恢复到合作状态\n\n";
    }

private:
    // 辅助函数：打印表格分隔线
    void printTableDivider(int nameWidth, int scoreWidth, int cols) const {
        std::cout << "  +";
        std::cout << std::string(nameWidth, '-') << "+";
        for (int i = 0; i < cols; ++i) {
            std::cout << std::string(scoreWidth, '-') << "+";
        }
        std::cout << "\n";
    }

    // 辅助函数：截断或填充字符串到指定宽度
    std::string fitString(const std::string& str, int width) const {
        if (str.length() > static_cast<size_t>(width - 1)) {
            return str.substr(0, width - 4) + "...";
        }
        return str;
    }

    void printMatchTable(const std::vector<StrategyPtr>& strategies,
        const std::vector<std::vector<std::pair<double, double>>>& matchResults) const {
        const int nameWidth = 20;
        const int scoreWidth = 18;

        std::cout << "\n对战结果矩阵 (ε = " << std::fixed << std::setprecision(2) 
                  << noise_level << "):\n";
        std::cout << "格式: P1得分 vs P2得分\n\n";

        // 打印表头
        printTableDivider(nameWidth, scoreWidth, strategies.size());
        std::cout << "  |" << std::setw(nameWidth) << std::left << " 策略 \\ 对手";
        for (const auto& s : strategies) {
            std::string name = fitString(s->getName(), scoreWidth);
            std::cout << "|" << std::setw(scoreWidth) << std::left << " " + name;
        }
        std::cout << "|\n";
        printTableDivider(nameWidth, scoreWidth, strategies.size());

        // 打印每一行
        for (size_t i = 0; i < strategies.size(); ++i) {
            std::string rowName = fitString(strategies[i]->getName(), nameWidth);
            std::cout << "  |" << std::setw(nameWidth) << std::left << " " + rowName;

            for (size_t j = 0; j < strategies.size(); ++j) {
                std::cout << "|";
                if (i == j) {
                    // 对角线：自己对自己
                    std::cout << std::setw(scoreWidth) << std::right
                        << std::fixed << std::setprecision(1)
                        << matchResults[i][j].first << " ";
                }
                else {
                    // 显示 P1 vs P2 得分
                    std::ostringstream oss;
                    oss << std::fixed << std::setprecision(1)
                        << matchResults[i][j].first << " : "
                        << matchResults[i][j].second;
                    std::cout << std::setw(scoreWidth) << std::right << oss.str() << " ";
                }
            }
            std::cout << "|\n";
        }
        printTableDivider(nameWidth, scoreWidth, strategies.size());
        std::cout << "\n";
    }
};

#endif // SIMULATOR_H