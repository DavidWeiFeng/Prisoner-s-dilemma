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
#include <cmath>
#include <tabulate/table.hpp>

using ScorePair = std::pair<double, double>;
using StrategyPtr = std::unique_ptr<Strategy>;

inline std::string moveToString(Move m) {
    return m == Move::Cooperate ? "C (Cooperate)" : "D (Defect)";
}

// Structure to hold statistical information about scores
struct ScoreStats {
    double mean;
    double stdev;
    double ci_lower;
    double ci_upper;
    int n_samples;

    ScoreStats() : mean(0.0), stdev(0.0), ci_lower(0.0), ci_upper(0.0), n_samples(0) {}
    
    ScoreStats(double m, double sd, double ci_low, double ci_high, int n)
        : mean(m), stdev(sd), ci_lower(ci_low), ci_upper(ci_high), n_samples(n) {}
};

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

    // Calculate mean and standard deviation from a vector of scores
    std::pair<double, double> calculateStats(const std::vector<double>& scores) const {
        if (scores.empty()) return {0.0, 0.0};
        
        double sum = 0.0;
        for (double s : scores) sum += s;
        double mean = sum / scores.size();
        
        if (scores.size() < 2) return {mean, 0.0};
        
        double variance = 0.0;
        for (double s : scores) {
            variance += (s - mean) * (s - mean);
        }
        variance /= (scores.size() - 1); // Sample variance (unbiased estimator)
        double stdev = std::sqrt(variance);
        
        return {mean, stdev};
    }
    
    // Calculate 95% confidence interval
    // Formula: mean ± 1.96 × (stdev / √n)
    ScoreStats calculateConfidenceInterval(const std::vector<double>& scores) const {
        if (scores.empty()) return ScoreStats();
        
        auto [mean, stdev] = calculateStats(scores);
        
        if (scores.size() < 2) {
            return ScoreStats(mean, stdev, mean, mean, scores.size());
        }
        
        double se = stdev / std::sqrt(scores.size()); // Standard Error
        double margin = 1.96 * se; // 95% CI uses z = 1.96
        
        return ScoreStats(mean, stdev, mean - margin, mean + margin, scores.size());
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

    // 标准锦标赛 with confidence intervals
    std::map<std::string, ScoreStats> runTournament(const std::vector<StrategyPtr>& strategies, 
                                                      int rounds, int repeats) const {
        std::map<std::string, std::vector<double>> allScores; // Track all scores for CI calculation
        std::map<std::string, int> matchCounts;

        // 初始化
        for (const auto& s : strategies) {
            allScores[s->getName()] = std::vector<double>();
            matchCounts[s->getName()] = 0;
        }

        // 存储详细对战结果用于表格显示
        int N = strategies.size();
        std::vector<std::vector<ScorePair>> matchResults(N, std::vector<ScorePair>(N));


        // 循环赛：每个策略两两对战
        for (size_t i = 0; i < strategies.size(); ++i) {
            for (size_t j = i; j < strategies.size(); ++j) {
                const auto& p1 = strategies[i];
                const auto& p2 = strategies[j];
                
                std::vector<double> p1_scores;
                std::vector<double> p2_scores;

                for (int r = 0; r < repeats; ++r) {
                    ScorePair scores = runGame(p1, p2, rounds);
                    p1_scores.push_back(scores.first);
                    p2_scores.push_back(scores.second);
                    
                    // Store for overall statistics
                    // 修复: 当策略对战自己时(i==j)，只添加一次分数
                    if (i == j) {
                        // 同一策略对战自己，两个分数相同，只添加一次
                        allScores[p1->getName()].push_back(scores.first);
                    } else {
                        // 不同策略对战，分别添加各自的分数
                        allScores[p1->getName()].push_back(scores.first);
                        allScores[p2->getName()].push_back(scores.second);
                    }
                }

                // Calculate average for match table
                double avg_score1 = 0.0, avg_score2 = 0.0;
                for (double s : p1_scores) avg_score1 += s;
                for (double s : p2_scores) avg_score2 += s;
                avg_score1 /= repeats;
                avg_score2 /= repeats;

                matchResults[i][j] = { avg_score1, avg_score2 };
                if (i != j) {
                    matchResults[j][i] = { avg_score2, avg_score1 };
                }

                matchCounts[p1->getName()] += repeats;
                if (i != j) {
                    matchCounts[p2->getName()] += repeats;
                }
            }
        }

        // 打印对战结果表格
        printMatchTable(strategies, matchResults);

        // 计算每个策略的总体统计信息（包括置信区间）
        std::map<std::string, ScoreStats> stats;
        for (const auto& [name, scores] : allScores) {
            stats[name] = calculateConfidenceInterval(scores);
        }
        
        return stats;
    }

    // Noise Sweep: Run tournaments at different noise levels
    std::map<double, std::map<std::string, ScoreStats>> runNoiseSweep(
        std::vector<StrategyPtr>& strategies,
        int rounds,
        int repeats,
        const std::vector<double>& noise_levels) const {

        std::map<double, std::map<std::string, ScoreStats>> results;

        std::cout << "\n=================================================\n";
        std::cout << "       Noise Sweep Experiment\n";
        std::cout << "=================================================\n\n";

        for (double epsilon : noise_levels) {
            std::cout << "\n--- Testing noise level ε = " << std::fixed << std::setprecision(2)
                << epsilon << " ---\n";

            // Set noise for all strategies
            for (auto& s : strategies) {
                s->setNoise(epsilon);
            }

            // Run the tournament
            std::map<std::string, ScoreStats> tournamentResults = runTournament(strategies, rounds, repeats);
            results[epsilon] = tournamentResults;

            // Print results for this noise level
            std::cout << "\nAverage scores at noise ε = " << epsilon << " (with 95% CI):\n";
            std::vector<std::pair<std::string, ScoreStats>> sorted_results(
                tournamentResults.begin(), tournamentResults.end());
            std::sort(sorted_results.begin(), sorted_results.end(),
                [](const auto& a, const auto& b) { return a.second.mean > b.second.mean; });

            for (const auto& [name, stats] : sorted_results) {
                std::cout << "  " << std::setw(15) << std::left << name << ": "
                    << std::fixed << std::setprecision(2) << stats.mean 
                    << "  [" << stats.ci_lower << ", " << stats.ci_upper << "]\n";
            }
        }

        return results;
    }

    // 打印噪声扫描结果表格
    static void printNoiseSweepTable(const std::map<double, std::map<std::string, ScoreStats>>& results) {
        if (results.empty()) return;

        std::cout << "\n=================================================\n";
        std::cout << "   噪声扫描结果汇总表 (Noise Sweep Summary)\n";
        std::cout << "=================================================\n\n";

        // 获取所有策略名称
        std::vector<std::string> strategies;
        for (const auto& [name, stats] : results.begin()->second) {
            strategies.push_back(name);
        }

        // 打印表头
        std::cout << std::setw(10) << "ε (Noise)";
        for (const auto& name : strategies) {
            std::cout << std::setw(25) << name;
        }
        std::cout << "\n";
        std::cout << std::string(10 + strategies.size() * 25, '-') << "\n";

        // 打印每个噪声水平的结果（mean ± CI）
        for (const auto& [epsilon, scores] : results) {
            std::cout << std::fixed << std::setprecision(2) << std::setw(10) << epsilon;
            for (const auto& name : strategies) {
                const auto& stats = scores.at(name);
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2) 
                    << stats.mean << " [" << stats.ci_lower << "," << stats.ci_upper << "]";
                std::cout << std::setw(25) << oss.str();
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    // 分析噪声对策略的影响
    static void analyzeNoiseImpact(const std::map<double, std::map<std::string, ScoreStats>>& results) {
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
        for (const auto& [name, base_stats] : baseline) {
            double base_score = base_stats.mean;
            double high_score = highest.at(name).mean;
            double drop_percent = ((base_score - high_score) / base_score) * 100.0;
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
    void printMatchTable(const std::vector<StrategyPtr>& strategies,
        const std::vector<std::vector<std::pair<double, double>>>& matchResults) const {

        std::cout << "\n--- Match Result Matrix (Noise ε = "
            << std::fixed << std::setprecision(2) << noise_level << ") ---\n";
        std::cout << "Format: P1 score : P2 score\n\n";

        tabulate::Table table;

        // 表头
        std::vector<std::string> header = { "Strategy \\ Opponent" };
        for (const auto& s : strategies) {
            header.push_back(s->getName());
        }
        table.add_row({ header.begin(), header.end() }); 


        // 填充每一行
        for (size_t i = 0; i < strategies.size(); ++i) {
            std::vector<std::string> row;
            row.push_back(strategies[i]->getName());  // 行标题

            for (size_t j = 0; j < strategies.size(); ++j) {
				std::ostringstream oss;
				oss << std::fixed << std::setprecision(2);
                if (i == j) {
                    // 对角线：自己 vs 自己
					oss << matchResults[i][j].first;
                }
                else {
                    // P1 vs P2
                    oss << matchResults[i][j].first << " : " << matchResults[i][j].second;
                }
				row.push_back(oss.str());
            }
            table.add_row({row.begin(),row.end()});
        }

        // 格式化表格
        table.format()
            .border_color(tabulate::Color::green)
            .font_align(tabulate::FontAlign::center);

        std::cout << table << std::endl;
    }
};

#endif // SIMULATOR_H