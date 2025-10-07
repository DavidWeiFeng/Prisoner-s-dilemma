#include "SimulatorRunner.h"
#include "Strategies.h" // 包含具体的策略实现
#include "CLI.hpp"      // 假设使用 CLI11 库

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <vector>

// 构造函数使用配置中的收益来初始化模拟器。
SimulatorRunner::SimulatorRunner(const Config& config)
    : config_(config), simulator_(config.payoffs) {
}

// 主要执行流程
void SimulatorRunner::run() {
    setupStrategies();
    printConfiguration();
    runSimulation();
    printResults();
}

// 从策略名称创建策略实例的中心位置。
std::unique_ptr<Strategy> SimulatorRunner::createStrategy(const std::string& name) {
    if (name == "AllCooperate") return std::make_unique<AllCooperate>();
    if (name == "AllDefect") return std::make_unique<AllDefect>();
    if (name == "TitForTat") return std::make_unique<TitForTat>();
    if (name == "GrimTrigger") return std::make_unique<GrimTrigger>();
    if (name == "PAVLOV") return std::make_unique<PAVLOV>();
    if (name == "RandomStrategy") return std::make_unique<RandomStrategy>();
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
        strategies_.push_back(std::move(strat));
    }

    if (strategies_.size() < 2) {
        throw std::runtime_error("一场锦标赛至少需要两种策略。");
    }
}

// 打印模拟配置的详细信息。
void SimulatorRunner::printConfiguration() const {
    std::cout << "\n--- 模拟配置 ---\n"
        << "每场比赛的回合数: " << config_.rounds << "\n"
        << "每场比赛的重复次数: " << config_.repeats << "\n"
        << "噪声（Epsilon）: " << config_.epsilon << "\n"
        << "随机种子: " << config_.seed << "\n"
        << "收益 (T,R,P,S): " << std::fixed << std::setprecision(1)
        << config_.payoffs[0] << "," << config_.payoffs[1] << ","
        << config_.payoffs[2] << "," << config_.payoffs[3] << "\n"
        << "参赛策略 (" << strategies_.size() << "个): ";
    for (const auto& s : strategies_) {
        std::cout << s->getName() << " | ";
    }
    std::cout << "\n";

    if (config_.evolve) {
        std::cout << "\n--- 进化参数 ---\n"
            << "种群数量: " << config_.population << "\n"
            << "进化代数: " << config_.generations << "\n"
            << "突变率: " << std::fixed << std::setprecision(2) << config_.mutation << "\n";
    }
}

// 运行相应的模拟（标准锦标赛或进化模拟）。
void SimulatorRunner::runSimulation() {
    std::cout << "\n--- 锦标赛开始 ---\n";
    if (config_.evolve) {
        // 注意：进化逻辑尚未在 GameEngine 中实现。
        // 这里只是一个占位符，告诉您应该在哪里调用它。
        std::cout << "已选择进化模式（功能尚未实现）。\n";
        // results_ = simulator_.runEvolution(...);
    }
    else {
        results_ = simulator_.runTournament(strategies_, config_.rounds, config_.repeats);
    }
}

// 打印最终结果。
void SimulatorRunner::printResults() const {
    std::cout << "\n--- 锦标赛结果 (各策略平均分) ---\n";
    // TODO: 根据 config_.format 实现不同的输出格式 (text, csv, json)
    for (const auto& [name, score] : results_) {
        std::cout << std::setw(30) << std::left << name << ": "
            << std::fixed << std::setprecision(4) << score << "\n";
    }
    std::cout << "\n--- 模拟结束 ---\n";
}



// 处理所有命令行解析并返回一个 Config 结构体。
Config SimulatorRunner::parseArguments(int argc, char** argv) {
    CLI::App app{ "迭代囚徒困境模拟器" };
    Config config;

    // 设置 CLI 选项并将它们绑定到 config 结构体的成员上。
    app.add_option("--rounds", config.rounds, "每场比赛的回合数。");
    app.add_option("--repeats", config.repeats, "为计算平均值，每场比赛重复的次数。");
    app.add_option("--epsilon", config.epsilon, "随机行动（错误率）的概率。");
    app.add_option("--seed", config.seed, "用于可复现性的随机种子。");
    app.add_option("--payoffs", config.payoffs, "收益值 [T, R, P, S]。")->expected(4);
    app.add_option("--strategies", config.strategy_names, "参赛策略列表。");
    app.add_option("--format", config.format, "输出格式 (text, csv, json)。");
    app.add_option("--save", config.save_file, "保存配置的文件路径。");
    app.add_option("--load", config.load_file, "加载配置的文件路径。");
    app.add_flag("--evolve", config.evolve, "启用进化模拟模式。");
    app.add_option("--population", config.population, "进化模拟的种群大小。");
    app.add_option("--generations", config.generations, "进化模拟的代数。");
    app.add_option("--mutation", config.mutation, "进化模拟的突变率。");

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError& e) {
        // CLI::App 的 `exit` 方法会打印错误和帮助信息。
        // 我们重新抛出一个标准异常，以便在 main 函数中捕获。
        // 这样做可以避免直接退出，让 main 函数决定如何处理错误。
        throw std::runtime_error("命令行参数解析错误。请检查您的输入。");
    }

    return config;
}

