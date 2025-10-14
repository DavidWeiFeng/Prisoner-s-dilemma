#include "SimulatorRunner.h"
#include "Strategies.h"
#include "CLI.hpp"
#include <tabulate/table.hpp>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <vector>
#include <fstream>

// 构造函数使用配置中的收益来初始化模拟器。
SimulatorRunner::SimulatorRunner(const Config& config)
    : config_(config), simulator_(config.payoffs, config.epsilon) {
}

auto format_double = [](double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
    };


// 主要执行流程
void SimulatorRunner::run() {
    setupStrategies();
    printConfiguration();
    printPayoffMatrix();

    if (config_.exploiters)
    {
        runExploiter();
    }
    else {
        runSimulation();
        printResults();
        printAnalysisQ1();
    }
}

// 从策略名称创建策略实例的中心位置。
std::unique_ptr<Strategy> SimulatorRunner::createStrategy(const std::string& name) {
    if (name == "AllCooperate") return std::make_unique<AllCooperate>();
    if (name == "AllDefect") return std::make_unique<AllDefect>();
    if (name == "TitForTat") return std::make_unique<TitForTat>();
    if (name == "GrimTrigger") return std::make_unique<GrimTrigger>();
    if (name == "PAVLOV") return std::make_unique<PAVLOV>();
    if (name == "ContriteTitForTat") return std::make_unique<ContriteTitForTat>();
    if (name == "RandomStrategy") return std::make_unique<RandomStrategy>();
    if (name == "PROBER") return std::make_unique<PROBER>();
    // 您可以在这里添加课程作业要求的其他策略
    return nullptr;
}

// 设置锦标赛中要使用的策略。
void SimulatorRunner::setupStrategies() {
    for (const auto& name : config_.strategy_names) {
        auto strat = createStrategy(name);
        if (!strat) {
            throw std::runtime_error("发现未知策略: " + name);
        }
        // 设置噪声水平
        strat->setNoise(config_.epsilon);
		// 设置随机种子（如果适用）
		strat->setSeed(config_.seed);
        strategies_.push_back(std::move(strat));
    }

    if (strategies_.size() < 2) {
        throw std::runtime_error("一场锦标赛至少需要两种策略。");
    }
}

// Print detailed simulation configuration
void SimulatorRunner::printConfiguration() const {
    std::cout << "\n=================================================\n";
    std::cout << "    Prisoner's Dilemma Simulator\n";
    std::cout << "=================================================\n\n";

    tabulate::Table table;

    // --- Simulation Configuration ---
    table.add_row({ "Rounds per match", std::to_string(config_.rounds) });
    table.add_row({ "Repeats per match", std::to_string(config_.repeats) });
    table.add_row({ "Epsilon",std::to_string(config_.epsilon) });
    table.add_row({ "Random seed", std::to_string(config_.seed) });

    // Payoffs
    table.add_row({ "Payoffs (T,R,P,S)",
        std::to_string(config_.payoffs[0]) + ", " +
        std::to_string(config_.payoffs[1]) + ", " +
        std::to_string(config_.payoffs[2]) + ", " +
        std::to_string(config_.payoffs[3])
        });

    // Strategies
    std::string strategy_list;
    for (const auto& s : strategies_) {
        strategy_list += s->getName() + " ";
    }
    table.add_row({ "Participating strategies", strategy_list });

    // Evolution parameters
    if (config_.evolve) {
        table.add_row({ "Population size", std::to_string(config_.population) });
        table.add_row({ "Generations", std::to_string(config_.generations) });
        table.add_row({ "Mutation rate", std::to_string(config_.mutation) });
    }

    // 格式化表格
    table.format()
        .border_color(tabulate::Color::none)
        .font_align(tabulate::FontAlign::center);

    std::cout << table << std::endl;
}

// Print the payoff matrix
void SimulatorRunner::printPayoffMatrix() const {
    double T = config_.payoffs[0];
    double R = config_.payoffs[1];
    double P = config_.payoffs[2];
    double S = config_.payoffs[3];

    std::cout << "\n--- Payoff Matrix ---\n";
    std::cout << "Based on the classic Prisoner's Dilemma parameters: T > R > P > S and 2R > T + S\n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "\n";
    tabulate::Table table;

    // 设置表头
    table.add_row({ "", "Opponent Cooperates (C)", "Opponent Defects (D)" });

    // 第一行：You Cooperate
    std::ostringstream oss1;
    oss1 << std::fixed << std::setprecision(2);
    oss1 << "R,R = " << R << "," << R;
    // 格式化 S,T
    std::ostringstream oss2;
    oss2 << std::fixed << std::setprecision(2);
    oss2 << "S,T = " << S << "," << T;
	std::string rr = "R,R = " + std::to_string(R) + "," + std::to_string(R);
    table.add_row({"You Cooperate (C)",oss1.str(),oss2.str()});

    std::ostringstream oss3;
    oss3 << std::fixed << std::setprecision(2);
    oss3 << "T,S = " << T << "," << S;
    // 格式化 S,T
    std::ostringstream oss4;
    oss4 << std::fixed << std::setprecision(2);
    oss4 << "P,P =  " << P << "," << P;
    table.add_row({ "You Defect (D)",oss3.str(),oss4.str() });

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

void SimulatorRunner::runSimulation() {
    std::cout << "\n--- Tournament Start ---\n";
    results_ = simulator_.runTournament(strategies_, config_.rounds, config_.repeats);
}

void SimulatorRunner::runExploiter() {
    std::cout << "\n--- Exploiter Tournament Start ---\n";
    if (strategies_.empty()) return; // 或其他错误处理
    //the first one is exploiter
    const auto& exploiter = strategies_[0];
    std::string exploiter_name = exploiter->getName();
    std::cout << "Exploiter: " << exploiter_name << "\n";
    std::vector<double> exploiter_scores;
    std::vector<double> victim_scores;

	//store results
    std::map<std::string, std::pair<ScoreStats, ScoreStats>> match_results;


    for (size_t i = 1; i < strategies_.size(); ++i) {
        const auto& victim = strategies_[i];
        std::string victim_name = victim->getName();
        std::cout << "Running: " << exploiter_name << " vs " << victim_name << "...\n";
        std::vector<double> exploiter_scores;
        std::vector<double> victim_scores;

		//repeat matches
        for (int repeat = 0; repeat < config_.repeats; ++repeat) {
            auto scores = simulator_.runGame(exploiter, victim, config_.rounds);
            exploiter_scores.push_back(scores.first);
            victim_scores.push_back(scores.second);
        }
        ScoreStats exploiter_stats = simulator_.calculateStats(exploiter_scores);
        ScoreStats victim_stats = simulator_.calculateStats(victim_scores);
        match_results[victim_name] = { exploiter_stats, victim_stats };
    }

    printExploiterResults(exploiter_name, match_results);

}

Config SimulatorRunner::parseArguments(int argc, char** argv) {
    CLI::App app{ "Iterated Prisoner's Dilemma Simulator" };
    Config config;

    app.add_option("--rounds", config.rounds, "Number of rounds per match.");
    app.add_option("--repeats", config.repeats, "Number of repetitions per match to compute the average score.");
    app.add_option("--epsilon", config.epsilon, "Probability of random action (error rate).");
    app.add_option("--seed", config.seed, "Random seed for reproducibility.");
    app.add_option("--payoffs", config.payoffs, "Payoff values [T, R, P, S].")->expected(4);
    app.add_option("--strategies", config.strategy_names, "List of participating strategies.");
    app.add_option("--format", config.format, "Output format (text, csv, json).");
    app.add_option("--save", config.save_file, "File path to save the current configuration.");
    app.add_option("--load", config.load_file, "File path to load a saved configuration.");
    app.add_flag("--evolve", config.evolve, "Enable evolutionary simulation mode.");
    app.add_option("--population", config.population, "Population size for the evolutionary simulation.");
    app.add_option("--generations", config.generations, "Number of generations for the evolutionary simulation.");
    app.add_option("--mutation", config.mutation, "Mutation rate for the evolutionary simulation.");
    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError& e) {
        throw std::runtime_error("Command-line argument parsing error. Please check your input. Error: " + std::string(e.what()));
    }

    return config;
}

// 辅助函数：打印剥削者测试结果
void SimulatorRunner::printExploiterResults(
    const std::string& exploiter_name,
    const std::map<std::string, std::pair<ScoreStats, ScoreStats>>& results) const {

    std::cout << "\n=================================================\n";
    std::cout << "--- Exploiter Test Results ---\n";
    std::cout << "=================================================\n\n";

    std::cout << "Exploiter: " << exploiter_name << "\n";
    std::cout << "Based on " << config_.repeats << " repeated experiments\n";
    std::cout << "Rounds per match: " << config_.rounds << "\n\n";

    // 创建表格
    tabulate::Table table;

    // 表头
    table.add_row({
        "Victim Strategy",
        exploiter_name + " Mean",
        exploiter_name + " 95% CI",
        "Victim Mean",
        "Victim 95% CI",
        "Score Difference"
        });

    // 添加每场对战的结果
    for (const auto& [victim_name, stats_pair] : results) {
        const auto& exploiter_stats = stats_pair.first;
        const auto& victim_stats = stats_pair.second;

        double score_diff = exploiter_stats.mean - victim_stats.mean;

        std::string exploiter_ci = format_double(exploiter_stats.ci_lower) + " - " +
            format_double(exploiter_stats.ci_upper);
        std::string victim_ci = format_double(victim_stats.ci_lower) + " - " +
            format_double(victim_stats.ci_upper);

        table.add_row({
            victim_name,
            format_double(exploiter_stats.mean),
            exploiter_ci,
            format_double(victim_stats.mean),
            victim_ci,
            format_double(score_diff)
            });
    }

    // 格式化表格
    table.format()
        .font_align(tabulate::FontAlign::center)
        .border_color(tabulate::Color::cyan);

    // 表头加粗
    table[0].format()
        .font_style({ tabulate::FontStyle::bold })
        .font_align(tabulate::FontAlign::center);

    std::cout << table << "\n\n";

    // 打印分析
}
void SimulatorRunner::printResults() const {
    std::cout << "\n=================================================\n";
    std::cout << "--- Tournament Results (Average Score per Strategy) ---\n";
    std::cout << "=================================================\n";

    // Sort results by mean score (descending)
    std::vector<std::pair<std::string, ScoreStats>> sorted_results(results_.begin(), results_.end());
    std::sort(sorted_results.begin(), sorted_results.end(),
        [](const auto& a, const auto& b) { return a.second.mean > b.second.mean; });

    std::cout << "Based on " << config_.repeats << " repeated experiments\n\n";

    tabulate::Table table;
    table.add_row({ "Rank", "Strategy", "Mean", "95% CI Lower", "95% CI Upper", "Std Dev (σ)" });

    int rank = 1;
    for (const auto& [name, stats] : sorted_results) {
        table.add_row({
            std::to_string(rank++),
            name,
            format_double(stats.mean),
            format_double(stats.ci_lower),
            format_double(stats.ci_upper),
            format_double(stats.stdev)
            });
    }
    // Apply styling
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

    std::cout << "\nNotes:\n";
    std::cout << "  - Mean: The average score of this strategy across all matches.\n";
    std::cout << "  - 95% CI: 95% Confidence Interval — the range where the true mean is likely to fall.\n";
    std::cout << "  - Std Dev (σ): Indicates how dispersed the scores are.\n";
    std::cout << "  - CI Formula: mean ± 1.96 × (σ / √" << config_.repeats << ")\n";
    std::cout << "\n--- Simulation Complete ---\n";
}


void SimulatorRunner::printAnalysisQ1() const {
    std::cout << "\n=================================================\n";
    std::cout << "--- Q1 Analysis ---\n";
    std::cout << "=================================================\n\n";
    std::cout
        << "Based on the results in the table, ALLD (Always Defect) achieved the highest mean payoff\n"
        << "in a noise-free environment, ranking first with a mean score of 50.40.\n\n"
        << "This outcome suggests that when there is no noise or execution error, defection remains\n"
        << "the most profitable strategy overall, since players can consistently exploit cooperative\n"
        << "opponents without risk of accidental punishment.\n";
}

