#include "ResultsPrinter.h"
#include <tabulate/table.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>

ResultsPrinter::ResultsPrinter(const Config& config) : config_(config) {
}

// ==================== 辅助函数 ====================

std::string ResultsPrinter::formatDouble(double value) {
    return formatDouble(value, 2);
}

std::string ResultsPrinter::formatDouble(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

// ==================== 配置和矩阵打印 ====================

void ResultsPrinter::printConfiguration(const std::vector<std::unique_ptr<Strategy>>& strategies) const {
    std::cout << "\n=================================================\n";
    std::cout << "    Prisoner's Dilemma Simulator\n";
    std::cout << "=================================================\n\n";

    tabulate::Table table;

    // 模拟配置
    table.add_row({ "Rounds per match", std::to_string(config_.rounds) });
    table.add_row({ "Repeats per match", std::to_string(config_.repeats) });
    table.add_row({ "Epsilon", std::to_string(config_.epsilon) });
    table.add_row({ "Random seed", std::to_string(config_.seed) });

    // 收益值
    table.add_row({ "Payoffs (T,R,P,S)",
        std::to_string(config_.payoffs[0]) + ", " +
        std::to_string(config_.payoffs[1]) + ", " +
        std::to_string(config_.payoffs[2]) + ", " +
        std::to_string(config_.payoffs[3])
    });

    // 策略列表
    std::string strategy_list;
    for (const auto& s : strategies) {
        strategy_list += s->getName() + " ";
    }
    table.add_row({ "Participating strategies", strategy_list });

    // 进化参数
    if (config_.evolve) {
        table.add_row({ "Generations", std::to_string(config_.generations) });
    }

    // 格式化表格
    table.format()
        .border_color(tabulate::Color::none)
        .font_align(tabulate::FontAlign::center);

    std::cout << table << std::endl;
}

void ResultsPrinter::printPayoffMatrix() const {
    double T = config_.payoffs[0];
    double R = config_.payoffs[1];
    double P = config_.payoffs[2];
    double S = config_.payoffs[3];

    std::cout << "\n--- Payoff Matrix ---\n";
    std::cout << "Based on the classic Prisoner's Dilemma parameters: T > R > P > S and 2R > T + S\n";
    std::cout << "\n";
    
    tabulate::Table table;

    // 设置表头
    table.add_row({ "", "Opponent Cooperates (C)", "Opponent Defects (D)" });

    // 第一行：You Cooperate
    std::ostringstream oss1;
    oss1 << std::fixed << std::setprecision(2);
    oss1 << "R,R = " << R << "," << R;
    
    std::ostringstream oss2;
    oss2 << std::fixed << std::setprecision(2);
    oss2 << "S,T = " << S << "," << T;
    
    table.add_row({ "You Cooperate (C)", oss1.str(), oss2.str() });

    // 第二行：You Defect
    std::ostringstream oss3;
    oss3 << std::fixed << std::setprecision(2);
    oss3 << "T,S = " << T << "," << S;
    
    std::ostringstream oss4;
    oss4 << std::fixed << std::setprecision(2);
    oss4 << "P,P = " << P << "," << P;
    
    table.add_row({ "You Defect (D)", oss3.str(), oss4.str() });

    // 格式化表格
    table.format()
        .border_color(tabulate::Color::blue)
        .font_align(tabulate::FontAlign::center);

    std::cout << table << std::endl;
    
    std::cout << "\nWhere:\n";
    std::cout << "  T (Temptation) = " << T << "  - Temptation to defect against a cooperator\n";
    std::cout << "  R (Reward)     = " << R << "  - Reward for mutual cooperation\n";
    std::cout << "  P (Punishment) = " << P << "  - Punishment for mutual defection\n";
    std::cout << "  S (Sucker)     = " << S << "  - Payoff for being betrayed when cooperating\n";
    std::cout << "\n";
}

// ==================== 锦标赛结果打印 ====================

void ResultsPrinter::printTournamentResults(const std::map<std::string, ScoreStats>& results) const {
    std::cout << "\n=================================================\n";
    std::cout << "--- Tournament Results (Average Score per Strategy) ---\n";
    std::cout << "=================================================\n";

    // 按平均分排序
    std::vector<std::pair<std::string, ScoreStats>> sorted_results(results.begin(), results.end());
    std::sort(sorted_results.begin(), sorted_results.end(),
        [](const auto& a, const auto& b) { return a.second.mean > b.second.mean; });

    std::cout << "Based on " << config_.repeats << " repeated experiments\n\n";

    tabulate::Table table;
    table.add_row({ "Rank", "Strategy", "Mean", "95% CI Lower", "95% CI Upper", "Std Dev" });

    int rank = 1;
    for (const auto& [name, stats] : sorted_results) {
        table.add_row({
            std::to_string(rank++),
            name,
            formatDouble(stats.mean),
            formatDouble(stats.ci_lower),
            formatDouble(stats.ci_upper),
            formatDouble(stats.stdev)
        });
    }
    
    // 应用样式
    table.format()
        .font_align(tabulate::FontAlign::center)
        .font_style({ tabulate::FontStyle::bold })
        .border_color(tabulate::Color::cyan);

    for (size_t i = 0; i < table.size(); ++i) {
        table[i].format()
            .font_style({ tabulate::FontStyle::bold })
            .font_align(tabulate::FontAlign::center);
    }

    std::cout << table << "\n";
}

void ResultsPrinter::printMatchTable(
    const std::vector<std::unique_ptr<Strategy>>& strategies,
    const std::vector<std::vector<std::pair<double, double>>>& matchResults) const {

    std::cout << "\n--- Match Result Matrix" << std::endl;
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

// ==================== 噪声分析打印 ====================

void ResultsPrinter::printNoiseSweepTable(const std::map<double, std::map<std::string, ScoreStats>>& results) const {
    if (results.empty()) return;

    std::cout << "\n=================================================\n";
    std::cout << " Noise Sweep Summary\n";
    std::cout << "=================================================\n\n";

    // 获取所有策略名称
    std::vector<std::string> strategies;
    for (const auto& [name, stats] : results.begin()->second) {
        strategies.push_back(name);
    }

    // 打印表头
    std::cout << std::setw(10) << "  (Noise)";
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

void ResultsPrinter::printNoiseAnalysisTable(
    const std::map<double, std::map<std::string, ScoreStats>>& noise_results) const {
    
    std::cout << "\n=================================================\n";
    std::cout << "--- Noise Sweep Analysis Results ---\n";
    std::cout << "=================================================\n\n";
    
    // 收集所有策略名称
    std::vector<std::string> strategy_names;
    if (!noise_results.empty()) {
        for (const auto& [strategy, _] : noise_results.begin()->second) {
            strategy_names.push_back(strategy);
        }
    }
    
    // 创建表格
    tabulate::Table table;
    
    // 表头
    std::vector<std::string> header = {"Epsilon (epsilon)"};
    for (const auto& name : strategy_names) {
        header.push_back(name);
    }
    table.add_row({header.begin(), header.end()});
    
    // 表头样式
    table[0].format()
        .font_style({tabulate::FontStyle::bold})
        .font_align(tabulate::FontAlign::center)
        .font_color(tabulate::Color::yellow);
    
    // 数据行
    for (const auto& [epsilon, results] : noise_results) {
        std::vector<std::string> row;
        row.push_back(formatDouble(epsilon, 2));
        
        for (const auto& name : strategy_names) {
            if (results.find(name) != results.end()) {
                row.push_back(formatDouble(results.at(name).mean));
            } else {
                row.push_back("N/A");
            }
        }
        
        table.add_row({row.begin(), row.end()});
    }
    
    // 表格样式
    table.format()
        .font_align(tabulate::FontAlign::center)
        .border_color(tabulate::Color::cyan);
    
    std::cout << table << "\n\n";
    
    // 打印观察结果
    std::cout << "Observations:\n";
    std::cout << "  - Compare how each strategy's average payoff changes with noise level\n";
    std::cout << "  - Strategies with smaller drops are more noise-robust\n";
    std::cout << "  - Look for strategies that collapse (e.g., GRIM typically drops sharply)\n";
    std::cout << "  - CTFT and PAVLOV usually show better resilience to noise\n\n";
}

void ResultsPrinter::exportNoiseAnalysisToCSV(
    const std::map<double, std::map<std::string, ScoreStats>>& noise_results,
    const std::string& filename) const {
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << " for writing.\n";
        return;
    }
    
    // CSV 表头
    file << "Epsilon,Strategy,Mean,StdDev,CI_Lower,CI_Upper\n";
    
    // 写入数据
    for (const auto& [epsilon, results] : noise_results) {
        for (const auto& [strategy, stats] : results) {
            file << formatDouble(epsilon, 2) << ","
                 << strategy << ","
                 << formatDouble(stats.mean) << ","
                 << formatDouble(stats.stdev) << ","
                 << formatDouble(stats.ci_lower) << ","
                 << formatDouble(stats.ci_upper) << "\n";
        }
    }
    
    file.close();
    std::cout << "Noise analysis exported to: " << filename << "\n";
}

// ==================== 剥削者模式打印 ====================

void ResultsPrinter::printExploiterMatchTable(
    const std::string& exploiter_name,
    const std::map<std::string, std::pair<double, double>>& matchAverages) const {

    std::cout << "\n=================================================\n";
    std::cout << "--- Exploiter vs Victims: Average Scores ---\n";
    std::cout << "=================================================\n\n";

    tabulate::Table table;

    // 表头
    table.add_row({
        "Victim Strategy",
        exploiter_name + " Score",
        "Victim Score",
        "Score Difference"
    });

    // 表头样式
    table[0].format()
        .font_style({ tabulate::FontStyle::bold })
        .font_align(tabulate::FontAlign::center)
        .font_color(tabulate::Color::yellow);

    // 添加每场对战的数据
    double total_exploiter_score = 0.0;
    double total_victim_score = 0.0;

    for (const auto& [victim_name, scores] : matchAverages) {
        double exploiter_score = scores.first;
        double victim_score = scores.second;
        double difference = exploiter_score - victim_score;

        total_exploiter_score += exploiter_score;
        total_victim_score += victim_score;

        table.add_row({
            victim_name,
            formatDouble(exploiter_score),
            formatDouble(victim_score),
            formatDouble(difference)
        });

        // 根据得分差异设置颜色
        size_t row_idx = table.size() - 1;
        if (difference > 50) {
            table[row_idx][3].format().font_color(tabulate::Color::green);
        }
        else if (difference > 0) {
            table[row_idx][3].format().font_color(tabulate::Color::yellow);
        }
        else {
            table[row_idx][3].format().font_color(tabulate::Color::red);
        }
    }

    // 添加总计行
    if (matchAverages.size() > 1) {
        double avg_exploiter = total_exploiter_score / matchAverages.size();
        double avg_victim = total_victim_score / matchAverages.size();
        double avg_difference = avg_exploiter - avg_victim;

        table.add_row({
            "Average",
            formatDouble(avg_exploiter),
            formatDouble(avg_victim),
            formatDouble(avg_difference)
        });

        // 总计行加粗
        table[table.size() - 1].format()
            .font_style({ tabulate::FontStyle::bold })
            .font_color(tabulate::Color::cyan);
    }

    // 设置表格样式
    table.format()
        .font_align(tabulate::FontAlign::center)
        .border_color(tabulate::Color::cyan)
        .border_top("═")
        .border_bottom("═")
        .border_left("║")
        .border_right("║");

    std::cout << table << "\n\n";

    // 打印解释
    std::cout << "Notes:\n";
    std::cout << "  - Each row shows the average score across " << config_.repeats << " matches.\n";
    std::cout << "  - Score Difference = " << exploiter_name << " Score - Victim Score\n";
    std::cout << "  - Positive difference (green/yellow) means " << exploiter_name << " is winning.\n";
    std::cout << "  - Negative difference (red) means the victim is resisting exploitation.\n\n";
}

void ResultsPrinter::showExploiterVsOpponent(
    const std::string& exploiter_name,
    const std::string& victim_name,
    const ScoreStats& exploiter_stats,
    const ScoreStats& victim_stats,
    int repeats,
    int rounds) const {

    std::cout << "\n=================================================\n";
    std::cout << "   Detailed Match: " << exploiter_name
        << " vs " << victim_name << "\n";
    std::cout << "=================================================\n\n";

    // 打印结果 - 使用 tabulate 库保持风格一致
    std::cout << "Results after " << repeats << " matches of " << rounds << " rounds:\n\n";

    tabulate::Table table;
    table.add_row({ "Strategy", "Mean Score", "95% CI Lower", "95% CI Upper", "Std Dev" });

    table[0].format()
        .font_style({ tabulate::FontStyle::bold })
        .font_align(tabulate::FontAlign::center)
        .font_color(tabulate::Color::yellow);

    table.add_row({
        exploiter_name,
        formatDouble(exploiter_stats.mean),
        formatDouble(exploiter_stats.ci_lower),
        formatDouble(exploiter_stats.ci_upper),
        formatDouble(exploiter_stats.stdev)
        });

    table.add_row({
        victim_name,
        formatDouble(victim_stats.mean),
        formatDouble(victim_stats.ci_lower),
        formatDouble(victim_stats.ci_upper),
        formatDouble(victim_stats.stdev)
        });

    table.format()
        .font_align(tabulate::FontAlign::center)
        .border_color(tabulate::Color::cyan);

    std::cout << table << "\n";
}

void ResultsPrinter::analyzeMixedPopulation(
    const std::map<std::string, ScoreStats>& results,
    const std::string& exploiter_name) const {
    
    std::cout << "\n=================================================\n";
    std::cout << "   Mixed Population Analysis: " << exploiter_name << "\n";
    std::cout << "=================================================\n\n";

    // 按得分排序
    std::vector<std::pair<std::string, ScoreStats>> sorted_results(
        results.begin(), results.end());
    std::sort(sorted_results.begin(), sorted_results.end(),
        [](const auto& a, const auto& b) { return a.second.mean > b.second.mean; });

    // 找到剥削者的排名
    int exploiter_rank = 0;
    int total_strategies = sorted_results.size();
    
    for (int i = 0; i < total_strategies; ++i) {
        if (sorted_results[i].first == exploiter_name) {
            exploiter_rank = i + 1;
            break;
        }
    }

    // 打印排名表
    std::cout << "Performance Ranking:\n\n";
    std::cout << std::setw(5) << "Rank" 
              << std::setw(15) << "Strategy" 
              << std::setw(12) << "Avg Score"
              << std::setw(25) << "95% CI"
              << "  Notes\n";
    std::cout << std::string(77, '-') << "\n";

    int rank = 1;
    for (const auto& [name, stats] : sorted_results) {
        std::cout << std::setw(5) << rank 
                  << std::setw(15) << name
                  << std::setw(12) << std::fixed << std::setprecision(2) << stats.mean
                  << "  [" << std::setw(6) << stats.ci_lower 
                  << "," << std::setw(6) << stats.ci_upper << "]";
        
        if (name == exploiter_name) {
            std::cout << "  ← EXPLOITER";
        }
        std::cout << "\n";
        rank++;
    }

    // 分析剥削者的表现
    std::cout << "\n--- Performance Analysis ---\n\n";
    
    auto exploiter_stats = results.at(exploiter_name);
    std::cout << exploiter_name << " finished in rank " << exploiter_rank 
              << " out of " << total_strategies << " strategies\n\n";

    if (exploiter_rank == 1) {
        std::cout << "✓ DOMINATES the population\n";
        std::cout << "  → High proportion of vulnerable strategies\n";
        std::cout << "  → Exploitation gains outweigh retaliation costs\n";
        std::cout << "  → This population is NOT stable (non-ESS)\n";
    } else if (exploiter_rank <= total_strategies / 2) {
        std::cout << "○ MODERATE performance\n";
        std::cout << "  → Successfully exploits some strategies\n";
        std::cout << "  → But punished by reciprocal strategies\n";
        std::cout << "  → Overall advantage is limited\n";
    } else {
        std::cout << "✗ POOR performance\n";
        std::cout << "  → Most strategies use retaliation\n";
        std::cout << "  → Trapped in mutual defection (P payoff)\n";
        std::cout << "  → Cannot compete with cooperative strategies\n";
        std::cout << "  → This is expected in diverse populations\n";
    }

    // 对比剥削者与最高分策略
    if (exploiter_rank > 1) {
        const auto& top_strategy = sorted_results[0];
        double score_gap = top_strategy.second.mean - exploiter_stats.mean;
        
        std::cout << "\nScore gap with leader (" << top_strategy.first << "): "
                  << std::fixed << std::setprecision(2) << score_gap << " points\n";
        std::cout << "  → Reciprocal strategies maintain cooperation among themselves\n";
        std::cout << "  → This generates higher average scores than indiscriminate defection\n";
    }

    // 理论解释
    std::cout << "\n--- Theoretical Insight ---\n\n";
    if (exploiter_name == "ALLD") {
        std::cout << "ALLD (Always Defect) in mixed populations:\n";
        std::cout << "  • Exploits unconditional cooperators (ALLC) → gains T payoff\n";
        std::cout << "  • But gets trapped in mutual defection with most others → receives P payoff\n";
        std::cout << "  • Since T > R > P > S, reciprocal strategies earning R outperform ALLD earning mostly P\n";
        std::cout << "  • Conclusion: Pure defection is NOT an Evolutionarily Stable Strategy (ESS)\n";
        std::cout << "                in populations with reciprocal strategies\n";
    } else if (exploiter_name == "PROBER") {
        std::cout << "PROBER in mixed populations:\n";
        std::cout << "  • Intelligently identifies exploitable targets (ALLC)\n";
        std::cout << "  • Switches to cooperation with defensive strategies (TFT, PAVLOV, CTFT)\n";
        std::cout << "  • More adaptive than ALLD, but success depends on population composition\n";
        std::cout << "  • Performance rank indicates the proportion of vulnerable vs. defensive strategies\n";
    }
    std::cout << "\n";
}

// ==================== 进化模拟打印 ====================

void ResultsPrinter::printEvolutionHeader() const {
    std::cout << "\n=================================================\n";
    std::cout << "    Evolutionary Tournament\n";
    std::cout << "=================================================\n\n";
}

void ResultsPrinter::printEvolutionHistory(
    const std::vector<std::map<std::string, double>>& history,
    const std::vector<std::unique_ptr<Strategy>>& strategies,
    const std::string& label) const {
    
    std::cout << "\n--- Evolution History (" << label << ") ---\n";
    
    tabulate::Table table;
    
    // 添加表头
    std::vector<std::string> header = { "Generation" };
    for (const auto& s : strategies) {
        header.push_back(s->getName());
    }
    table.add_row({ header.begin(), header.end() });
    
    // 添加数据行 - 每4代显示一次
    for (size_t gen = 0; gen < history.size(); gen++) {
        if (gen % 4 == 0 || gen == history.size() - 1) {
            std::vector<std::string> row = { std::to_string(gen) };
            for (const auto& s : strategies) {
                row.push_back(formatDouble(history[gen].at(s->getName()), 3));
            }
            table.add_row({ row.begin(), row.end() });
        }
    }
    
    // 格式化表格
    table.format()
        .font_align(tabulate::FontAlign::center)
        .border_color(tabulate::Color::cyan);
    
    // 表头加粗并使用黄色
    table[0].format()
        .font_style({ tabulate::FontStyle::bold });
    
    std::cout << table << "\n";
}

// ==================== SCB (Strategic Complexity Budget) 打印 ====================

void ResultsPrinter::printComplexityTable(const std::vector<std::unique_ptr<Strategy>>& strategies) const {
    std::cout << "\n=================================================\n";
    std::cout << "--- Strategy Complexity Table ---\n";
    std::cout << "=================================================\n\n";

    tabulate::Table table;

    // 表头
    table.add_row({ "Strategy", "Complexity Score", "Reason" });

    // 表头样式
    table[0].format()
        .font_style({ tabulate::FontStyle::bold })
        .font_align(tabulate::FontAlign::center)
        .font_color(tabulate::Color::yellow);

    // 添加每个策略的复杂度信息
    for (const auto& s : strategies) {
        table.add_row({
            s->getName(),
            formatDouble(s->getComplexity(), 1),
            s->getComplexityReason()
        });
    }

    // 格式化表格
    table.format()
        .font_align(tabulate::FontAlign::center)
        .border_color(tabulate::Color::cyan);

    std::cout << table << "\n\n";

    // 打印说明
    std::cout << "Complexity Score Interpretation:\n";
    std::cout << "  1.0 - Simplest strategies (no memory, fixed output)\n";
    std::cout << "  2.0 - Basic memory-based strategies (1-round memory)\n";
    std::cout << "  2.5 - Moderate complexity (state tracking + logic)\n";
    std::cout << "  3.5 - High complexity (multi-round memory, noise handling, probing)\n\n";
    
    if (Strategy::isSCBEnabled()) {
        std::cout << "SCB Status: ENABLED\n";
        std::cout << "Cost Factor: " << formatDouble(Strategy::getSCBCostFactor(), 2) 
                  << " per complexity unit per round\n";
        std::cout << "Formula: adjusted_score = raw_score - (complexity × cost_factor × rounds)\n\n";
    } else {
        std::cout << "SCB Status: DISABLED\n\n";
    }
}

void ResultsPrinter::printSCBComparison(
    const std::map<std::string, ScoreStats>& results_without_scb,
    const std::map<std::string, ScoreStats>& results_with_scb) const {

    std::cout << "\n=================================================\n";
    std::cout << "--- Tournament Results Comparison (With/Without SCB) ---\n";
    std::cout << "=================================================\n\n";

    // 收集所有策略并按无 SCB 时的得分排序
    std::vector<std::string> strategy_names;
    for (const auto& [name, _] : results_without_scb) {
        strategy_names.push_back(name);
    }
    std::sort(strategy_names.begin(), strategy_names.end(),
        [&](const std::string& a, const std::string& b) {
            return results_without_scb.at(a).mean > results_without_scb.at(b).mean;
        });

    tabulate::Table table;

    // 表头
    table.add_row({
        "Strategy",
        "Without SCB",
        "Rank",
        "With SCB",
        "Rank",
        "Score Diff",
        "Rank Change"
    });

    // 表头样式
    table[0].format()
        .font_style({ tabulate::FontStyle::bold })
        .font_align(tabulate::FontAlign::center)
        .font_color(tabulate::Color::yellow);

    // 计算排名
    std::map<std::string, int> rank_without, rank_with;
    int r = 1;
    for (const auto& name : strategy_names) {
        rank_without[name] = r++;
    }

    // 按有 SCB 时的得分重新排序以计算排名
    std::vector<std::string> sorted_with_scb = strategy_names;
    std::sort(sorted_with_scb.begin(), sorted_with_scb.end(),
        [&](const std::string& a, const std::string& b) {
            return results_with_scb.at(a).mean > results_with_scb.at(b).mean;
        });
    r = 1;
    for (const auto& name : sorted_with_scb) {
        rank_with[name] = r++;
    }

    // 填充表格
    for (const auto& name : strategy_names) {
        double score_without = results_without_scb.at(name).mean;
        double score_with = results_with_scb.at(name).mean;
        double score_diff = score_with - score_without;
        int rank_change = rank_without[name] - rank_with[name];

        std::string rank_change_str;
        if (rank_change > 0) {
            rank_change_str = "↑" + std::to_string(rank_change);
        } else if (rank_change < 0) {
            rank_change_str = "↓" + std::to_string(-rank_change);
        } else {
            rank_change_str = "→";
        }

        table.add_row({
            name,
            formatDouble(score_without),
            std::to_string(rank_without[name]),
            formatDouble(score_with),
            std::to_string(rank_with[name]),
            formatDouble(score_diff),
            rank_change_str
        });

        // 根据排名变化设置颜色
        size_t row_idx = table.size() - 1;
        if (rank_change > 0) {
            table[row_idx][6].format().font_color(tabulate::Color::green);
        } else if (rank_change < 0) {
            table[row_idx][6].format().font_color(tabulate::Color::red);
        }
    }

    // 格式化表格
    table.format()
        .font_align(tabulate::FontAlign::center)
        .border_color(tabulate::Color::cyan);

    std::cout << table << "\n\n";

    std::cout << "Notes:\n";
    std::cout << "  - Cost factor = " << formatDouble(Strategy::getSCBCostFactor(), 2) 
              << " per complexity unit per round\n";
    std::cout << "  - Rounds per match = " << config_.rounds << "\n";
    std::cout << "  - ↑ indicates rank improvement, ↓ indicates rank decline\n";
    std::cout << "  - Negative Score Diff means complexity cost reduced the score\n\n";
}

void ResultsPrinter::printSCBAnalysis(
    const std::map<std::string, ScoreStats>& results_without_scb,
    const std::map<std::string, ScoreStats>& results_with_scb,
    const std::vector<std::unique_ptr<Strategy>>& strategies) const {

    std::cout << "\n=================================================\n";
    std::cout << "--- SCB Impact Analysis ---\n";
    std::cout << "=================================================\n\n";

    // 按复杂度分组策略
    std::map<double, std::vector<std::string>> complexity_groups;
    for (const auto& s : strategies) {
        complexity_groups[s->getComplexity()].push_back(s->getName());
    }

    // 计算每个策略的表现下降
    std::vector<std::tuple<std::string, double, double, double>> strategy_impacts;
    for (const auto& s : strategies) {
        std::string name = s->getName();
        double complexity = s->getComplexity();
        double score_without = results_without_scb.at(name).mean;
        double score_with = results_with_scb.at(name).mean;
        double drop_percent = ((score_without - score_with) / score_without) * 100.0;
        
        strategy_impacts.push_back({name, complexity, score_without - score_with, drop_percent});
    }

    // 按下降百分比排序
    std::sort(strategy_impacts.begin(), strategy_impacts.end(),
        [](const auto& a, const auto& b) {
            return std::get<3>(a) > std::get<3>(b);
        });

    std::cout << "Performance Impact (sorted by % drop):\n\n";
    
    for (const auto& [name, complexity, drop_absolute, drop_percent] : strategy_impacts) {
        std::cout << std::setw(12) << std::left << name << ": "
                  << "Complexity=" << std::fixed << std::setprecision(1) << complexity
                  << ", Drop=" << std::setprecision(2) << drop_absolute
                  << " (" << std::setprecision(1) << drop_percent << "%)";
        
        if (drop_percent > 15) {
            std::cout << "  [High Impact]";
        } else if (drop_percent > 10) {
            std::cout << "  [Moderate Impact]";
        } else {
            std::cout << "  [Low Impact]";
        }
        std::cout << "\n";
    }

    std::cout << "\n";
    std::cout << "Key Findings:\n\n";

    // 找出受影响最大和最小的策略
    auto most_impacted = strategy_impacts.front();
    auto least_impacted = strategy_impacts.back();

    std::cout << "1. Most Impacted Strategy:\n";
    std::cout << "   " << std::get<0>(most_impacted) 
              << " (Complexity=" << std::get<1>(most_impacted) << ")\n";
    std::cout << "   Lost " << formatDouble(std::get<2>(most_impacted)) 
              << " points (" << formatDouble(std::get<3>(most_impacted), 1) << "%)\n\n";

    std::cout << "2. Least Impacted Strategy:\n";
    std::cout << "   " << std::get<0>(least_impacted) 
              << " (Complexity=" << std::get<1>(least_impacted) << ")\n";
    std::cout << "   Lost " << formatDouble(std::get<2>(least_impacted)) 
              << " points (" << formatDouble(std::get<3>(least_impacted), 1) << "%)\n\n";

    std::cout << "3. Interpretation:\n";
    std::cout << "   When complexity carries a cost (factor=" 
              << formatDouble(Strategy::getSCBCostFactor(), 2) << "), ";
    
    // 检查简单策略是否相对排名上升
    bool simple_strategies_improved = false;
    for (const auto& [name, complexity, drop_abs, drop_pct] : strategy_impacts) {
        if (complexity <= 2.0 && drop_pct < 15.0) {
            simple_strategies_improved = true;
            break;
        }
    }
    
    if (simple_strategies_improved) {
        std::cout << "simple yet robust strategies\n";
        std::cout << "   gain relative advantage over sophisticated strategies.\n";
        std::cout << "   This suggests that in resource-constrained environments,\n";
        std::cout << "   **parsimony beats sophistication**.\n\n";
    } else {
        std::cout << "all strategies are proportionally\n";
        std::cout << "   penalized. The cost factor may need adjustment to observe\n";
        std::cout << "   meaningful shifts in strategy rankings.\n\n";
    }

    std::cout << "4. Trade-off Consideration:\n";
    std::cout << "   - High complexity strategies offer sophisticated behavior\n";
    std::cout << "     (e.g., noise detection, forgiveness, probing)\n";
    std::cout << "   - But they pay a cognitive/computational cost\n";
    std::cout << "   - Optimal strategy depends on the environment's cost structure\n\n";
}
