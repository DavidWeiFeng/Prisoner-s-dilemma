/**
 * @file OperatorOverloadingExample.cpp
 * @brief 演示如何使用 OutputExporter 中的运算符重载功能
 * 
 * 本文件展示了以下运算符重载的用法：
 * 1. Move 枚举的 I/O 运算符 (operator<< 和 operator>>)
 * 2. DoubleScoreStats 的比较运算符 (>, <, ==, >=, <=)
 * 3. DoubleScoreStats 的流输出运算符 (operator<<)
 * 4. LeaderboardEntry 的排序和输出运算符
 */

#include "OutputExporter.h"
#include "Simulator.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>

namespace OperatorOverloadingDemo {

/**
 * @brief 演示 Move 枚举的 I/O 运算符
 */
void demonstrateMoveIOOperators() {
    std::cout << "\n=== Move I/O Operators Demo ===\n";
    
    // 演示输出运算符
    Move cooperate = Move::Cooperate;
    Move defect = Move::Defect;
    
    std::cout << "Cooperate move: " << cooperate << "\n";
    std::cout << "Defect move: " << defect << "\n";
    
    // 演示输入运算符
    std::cout << "\n输入测试 (C 表示合作, D 表示背叛):\n";
    std::istringstream input_stream("C D c d");
    Move m1, m2, m3, m4;
    
    input_stream >> m1 >> m2 >> m3 >> m4;
    
    std::cout << "读取的动作: " << m1 << " " << m2 << " " << m3 << " " << m4 << "\n";
}

/**
 * @brief 演示 DoubleScoreStats 的比较运算符
 */
void demonstrateScoreStatsComparison() {
    std::cout << "\n=== DoubleScoreStats Comparison Operators Demo ===\n";
    
    // 创建一些测试统计数据
    DoubleScoreStats stats1(100.5, 5.2, 98.0, 103.0, 30);
    DoubleScoreStats stats2(95.3, 4.8, 93.0, 97.6, 30);
    DoubleScoreStats stats3(100.5, 5.0, 98.2, 102.8, 30);
    
    // 测试比较运算符
    std::cout << "stats1 mean: " << stats1.mean << "\n";
    std::cout << "stats2 mean: " << stats2.mean << "\n";
    std::cout << "stats3 mean: " << stats3.mean << "\n\n";
    
    std::cout << "stats1 > stats2: " << (stats1 > stats2 ? "true" : "false") << "\n";
    std::cout << "stats1 < stats2: " << (stats1 < stats2 ? "true" : "false") << "\n";
    std::cout << "stats1 == stats3: " << (stats1 == stats3 ? "true" : "false") << "\n";
    std::cout << "stats1 >= stats2: " << (stats1 >= stats2 ? "true" : "false") << "\n";
    std::cout << "stats2 <= stats1: " << (stats2 <= stats1 ? "true" : "false") << "\n";
    
    // 演示排序
    std::vector<DoubleScoreStats> scores = {stats2, stats1, stats3};
    std::cout << "\n排序前:\n";
    for (size_t i = 0; i < scores.size(); ++i) {
        std::cout << "  [" << i << "] mean: " << scores[i].mean << "\n";
    }
    
    std::sort(scores.begin(), scores.end(), std::greater<DoubleScoreStats>());
    
    std::cout << "\n降序排序后:\n";
    for (size_t i = 0; i < scores.size(); ++i) {
        std::cout << "  [" << i << "] mean: " << scores[i].mean << "\n";
    }
}

/**
 * @brief 演示 DoubleScoreStats 的流输出运算符
 */
void demonstrateScoreStatsOutput() {
    std::cout << "\n=== DoubleScoreStats Output Operator Demo ===\n";
    
    DoubleScoreStats stats1(105.7, 6.3, 103.2, 108.2, 50);
    DoubleScoreStats stats2(98.4, 5.1, 96.0, 100.8, 50);
    
    std::cout << "Strategy A: " << stats1 << "\n";
    std::cout << "Strategy B: " << stats2 << "\n";
}

/**
 * @brief 演示 LeaderboardEntry 的排序和输出
 */
void demonstrateLeaderboard() {
    std::cout << "\n=== Leaderboard Demo ===\n";
    
    // 创建排行榜条目
    std::vector<LeaderboardEntry> leaderboard;
    
    leaderboard.push_back(LeaderboardEntry(
        "Tit-for-Tat", 
        DoubleScoreStats(102.5, 5.0, 100.0, 105.0, 30), 
        0
    ));
    
    leaderboard.push_back(LeaderboardEntry(
        "Always Cooperate", 
        DoubleScoreStats(95.3, 4.2, 93.0, 97.6, 30), 
        0
    ));
    
    leaderboard.push_back(LeaderboardEntry(
        "Always Defect", 
        DoubleScoreStats(108.7, 6.1, 105.0, 112.4, 30), 
        0
    ));
    
    leaderboard.push_back(LeaderboardEntry(
        "Pavlov", 
        DoubleScoreStats(99.8, 5.5, 97.0, 102.6, 30), 
        0
    ));
    
    // 排序（使用 LeaderboardEntry 的 operator<）
    std::sort(leaderboard.begin(), leaderboard.end());
    
    // 分配排名
    for (size_t i = 0; i < leaderboard.size(); ++i) {
        leaderboard[i].rank = static_cast<int>(i + 1);
    }
    
    // 打印排行榜（使用 LeaderboardEntry 的 operator<<）
    std::cout << "\n最终排行榜:\n";
    std::cout << std::string(80, '-') << "\n";
    for (const auto& entry : leaderboard) {
        std::cout << entry << "\n";
    }
    std::cout << std::string(80, '-') << "\n";
}

/**
 * @brief 演示在实际场景中使用运算符重载
 */
void demonstratePracticalUsage() {
    std::cout << "\n=== Practical Usage Demo ===\n";
    
    // 模拟比赛结果
    std::map<std::string, DoubleScoreStats> tournament_results;
    tournament_results["Tit-for-Tat"] = DoubleScoreStats(102.5, 5.0, 100.0, 105.0, 30);
    tournament_results["Pavlov"] = DoubleScoreStats(99.8, 5.5, 97.0, 102.6, 30);
    tournament_results["Always Cooperate"] = DoubleScoreStats(95.3, 4.2, 93.0, 97.6, 30);
    tournament_results["Always Defect"] = DoubleScoreStats(108.7, 6.1, 105.0, 112.4, 30);
    tournament_results["Grim Trigger"] = DoubleScoreStats(101.2, 4.8, 98.5, 103.9, 30);
    
    // 转换为向量并使用运算符重载进行排序
    std::vector<std::pair<std::string, DoubleScoreStats>> results_vec(
        tournament_results.begin(), 
        tournament_results.end()
    );
    
    // 使用重载的 > 运算符排序
    std::sort(results_vec.begin(), results_vec.end(),
        [](const auto& a, const auto& b) { 
            return a.second > b.second;  // 使用 DoubleScoreStats 的 operator>
        }
    );
    
    // 显示结果
    std::cout << "\n比赛结果（按得分降序）:\n";
    std::cout << std::string(80, '=') << "\n";
    
    int rank = 1;
    for (const auto& [name, stats] : results_vec) {
        std::cout << "#" << rank++ << " " << std::setw(20) << std::left << name 
                  << " | " << stats << "\n";
    }
    
    // 使用流输出打印最优和最差策略
    std::cout << "\n最佳策略: " << results_vec.front().first << "\n";
    std::cout << "得分: " << results_vec.front().second << "\n";
    
    std::cout << "\n最差策略: " << results_vec.back().first << "\n";
    std::cout << "得分: " << results_vec.back().second << "\n";
}

/**
 * @brief 运行所有演示
 */
void runAllDemos() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║        运算符重载功能演示 (Operator Overloading Demo)         ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
    
    demonstrateMoveIOOperators();
    demonstrateScoreStatsComparison();
    demonstrateScoreStatsOutput();
    demonstrateLeaderboard();
    demonstratePracticalUsage();
    
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                      演示完成！                                ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
}

} // namespace OperatorOverloadingDemo

// 如果需要独立运行此演示，取消下面的注释
/*
int main() {
    OperatorOverloadingDemo::runAllDemos();
    return 0;
}
*/
