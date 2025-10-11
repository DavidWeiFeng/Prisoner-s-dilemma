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

// 主要执行流程
void SimulatorRunner::run() {
    setupStrategies();
    printConfiguration();
    printPayoffMatrix();
    
    // 检查是否运行噪声扫描
    if (config_.noise_sweep) {
        runNoiseSweep();
    } else {
        runSimulation();
        printResults();
        printAnalysis();
    }
}

// 从策略名称创建策略实例的中心位置。
std::unique_ptr<Strategy> SimulatorRunner::createStrategy(const std::string& name) {
    if (name == "AllCooperate") return std::make_unique<AllCooperate>();
    if (name == "AllDefect") return std::make_unique<AllDefect>();
    if (name == "TitForTat") return std::make_unique<TitForTat>();
    if (name == "GrimTrigger") return std::make_unique<GrimTrigger>();
    if (name == "PAVLOV") return std::make_unique<PAVLOV>();
    if (name == "CTFT") return std::make_unique<ContriteTitForTat>();
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

    if (config_.noise_sweep) {
        table.add_row({ "Noise sweep mode", "Enabled" });
        table.add_row({ "Noise range",
            std::to_string(config_.noise_min) + " to " +
            std::to_string(config_.noise_max) +
            " (step: " + std::to_string(config_.noise_step) + ")" });
    }
    else {
        table.add_row({ "Noise (Epsilon)", std::to_string(config_.epsilon) });
    }

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
    if (config_.evolve) {
        std::cout << "Evolution mode selected (feature not yet implemented).\n";
    }
    else {
        results_ = simulator_.runTournament(strategies_, config_.rounds, config_.repeats);
    }
}

// 打印最终结果。
void SimulatorRunner::printResults() const {
    std::cout << "\n=================================================\n";
    std::cout << "--- Tournament Results (Average Score per Strategy) ---\n";
    std::cout << "=================================================\n";

    // 创建一个排序的结果列表（按平均分排序）
    std::vector<std::pair<std::string, ScoreStats>> sorted_results(results_.begin(), results_.end());
    std::sort(sorted_results.begin(), sorted_results.end(),
        [](const auto& a, const auto& b) { return a.second.mean > b.second.mean; });

    std::cout << "\n格式: 策略名称: 平均分 [95% CI 下限, 上限] (标准差)\n";
    std::cout << "基于 " << config_.repeats << " 次重复实验\n\n";

    int rank = 1;
    for (const auto& [name, stats] : sorted_results) {
        std::cout << rank++ << ". " << std::setw(20) << std::left << name << ": "
            << std::fixed << std::setprecision(2) << stats.mean 
            << "  [" << stats.ci_lower << ", " << stats.ci_upper << "]"
            << "  (σ=" << stats.stdev << ")\n";
    }
    
    std::cout << "\n说明:\n";
    std::cout << "  - 平均分: 该策略在所有对战中的平均得分\n";
    std::cout << "  - 95% CI: 95%置信区间，真实均值有95%概率落在此区间内\n";
    std::cout << "  - 标准差(σ): 得分的离散程度\n";
    std::cout << "  - 置信区间公式: mean ± 1.96 × (σ / √" << config_.repeats << ")\n";
    
    std::cout << "\n--- 模拟结束 ---\n";
}

void SimulatorRunner::printAnalysis() const {
    std::cout << "\n=================================================\n";
    std::cout << "--- Strategy Analysis ---\n";
    std::cout << "=================================================\n\n";
}

void SimulatorRunner::runNoiseSweep() {
    std::vector<double> noise_levels;
    for (double eps = config_.noise_min; eps <= config_.noise_max + 0.001; eps += config_.noise_step) {
        noise_levels.push_back(eps);
    }

    auto results = simulator_.runNoiseSweep(strategies_, config_.rounds, config_.repeats, noise_levels);
    
    Simulator::printNoiseSweepTable(results);
    Simulator::analyzeNoiseImpact(results);
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

    app.add_flag("--noise-sweep", config.noise_sweep, "Enable noise sweep mode.");
    app.add_option("--noise-min", config.noise_min, "Minimum value for noise sweep.");
    app.add_option("--noise-max", config.noise_max, "Maximum value for noise sweep.");
    app.add_option("--noise-step", config.noise_step, "Step size for noise sweep.");

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError& e) {
        throw std::runtime_error("Command-line argument parsing error. Please check your input. Error: " + std::string(e.what()));
    }

    return config;
}
