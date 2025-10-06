#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <iomanip>
#include <stdexcept> // 包含 std::exception

// 包含所有自定义文件
#include "CLI.hpp"      // CLI 库 (现在使用标准的 CLI::App)
#include "Strategy.h"   // 策略基类
#include "Strategies.h" // 具体策略
#include "GameEngine.h" // 模拟器核心

using StrategyPtr = std::unique_ptr<Strategy>;
using CLI::App; // 使用真实的 CLI::App (或模拟的)

// 工厂方法
std::unique_ptr<Strategy> createStrategy(const std::string& name) {
    // 将所有策略注册到工厂
    if (name == "ALLC") return std::make_unique<AllCooperate>();
    if (name == "ALLD") return std::make_unique<AllDefect>();
    if (name == "TFT") return std::make_unique<TitForTat>();
    // TODO: 其他策略，如 STFT (Suspicious Tit for Tat)
    return nullptr;
}

int main(int argc, char** argv) {
    App app{ "Iterated Prisoner's Dilemma Simulator" };

    // --- 参数配置 ---
    int rounds = 150, repeats = 50, seed = 42;
    int population = 200, generations = 200;
    double epsilon = 0.0, mutation = 0.02;
    std::vector<double> payoffs = { 5.0, 3.0, 1.0, 0.0 }; // T, R, P, S (诱惑，奖励，惩罚，懦夫)
    std::vector<std::string> strategy_names = { "ALLC", "ALLD", "TFT" }; // 默认运行这三个策略
    std::string format = "text", save_file, load_file;
    bool evolve = false;

    // CLI 配置
    app.add_option("--rounds", rounds)->default_val(150)->description("Number of rounds per match.");
    app.add_option("--repeats", repeats)->default_val(50)->description("Number of times to repeat each match for averaging.");
    app.add_option("--epsilon", epsilon)->default_val(0.0)->description("Probability of a random move (error rate).");
    app.add_option("--seed", seed)->default_val(42)->description("Random seed for reproducibility.");
    app.add_option("--payoffs", payoffs)->expected(4)->default_val(payoffs)->description("Payoffs [T, R, P, S].");
    app.add_option("--strategies", strategy_names)->default_val(strategy_names)->description("List of strategies to compete (e.g., ALLC ALLD TFT).");
    app.add_option("--format", format)->default_val("text")->description("Output format (text or csv).");
    app.add_option("--save", save_file)->description("File path to save results.");
    app.add_option("--load", load_file)->description("File path to load previous data (not implemented).");
    app.add_flag("--evolve", evolve)->description("Enable evolutionary simulation mode.");
    app.add_option("--population", population)->default_val(200)->description("Population size for evolution.");
    app.add_option("--generations", generations)->default_val(200)->description("Number of generations for evolution.");
    app.add_option("--mutation", mutation)->default_val(0.02)->description("Mutation rate for evolution.");

    // --- 解析命令行参数 (使用标准 CLI11 模式) ---
    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError& e) {
        // CLI11 推荐的方式，处理帮助、版本请求或解析错误
        std::cerr << app.help_make_parser(e); // 打印帮助信息或错误信息 (模拟 App::help_make_parser)
        return e.exit_code;
    }
    catch (const std::exception& e) {
        std::cerr << "General Error during CLI parsing: " << e.what() << "\n";
        return 1;
    }

    // --- 策略加载 ---
    std::vector<StrategyPtr> strategies;
    for (auto& name : strategy_names) {
        auto strat = createStrategy(name);
        if (!strat) {
            std::cerr << "Unknown strategy: " << name << ". Exiting.\n";
            return 1;
        }
        strategies.push_back(std::move(strat));
    }

    if (strategies.size() < 2) {
        std::cerr << "Error: At least two strategies are required for a tournament.\n";
        return 1;
    }

    // --- 配置输出 ---
    std::cout << "\n--- Simulation Configuration ---\n"
        << "Rounds: " << rounds << "\n"
        << "Repeats: " << repeats << "\n"
        << "Epsilon: " << epsilon << "\n"
        << "Seed: " << seed << "\n"
        << "Payoffs (T,R,P,S): " << std::fixed << std::setprecision(1)
        << payoffs[0] << "," << payoffs[1] << ","
        << payoffs[2] << "," << payoffs[3] << "\n"
        << "Strategies (" << strategies.size() << "): ";
    for (const auto& s : strategies) std::cout << s->getName() << " | ";
    std::cout << "\n";

    if (evolve) {
        std::cout << "\n--- Evolutionary Parameters ---\n"
            << "Population: " << population << "\n"
            << "Generations: " << generations << "\n"
            << "Mutation: " << std::fixed << std::setprecision(2) << mutation << "\n";
    }

    // --- Simulation Starting ---
    std::cout << "\n--- Tournament Starting ---\n";

    // 实例化模拟器 (使用配置的得分)
    Simulator simulator(payoffs);

    // 运行循环赛
    std::map<std::string, double> results = simulator.runTournament(strategies, rounds, repeats);

    // --- 结果输出 ---
    std::cout << "\n--- Tournament Results (Avg Score per Strategy) ---\n";
    for (const auto& [name, score] : results) {
        std::cout << std::setw(30) << std::left << name << ": "
            << std::fixed << std::setprecision(4) << score << "\n";
    }

    std::cout << "--- Simulation Finished ---\n";

    return 0;
}
