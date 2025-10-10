#include "SimulatorRunner.h"
#include "Strategies.h" // 包含具体的策略实现
#include "CLI.hpp"      // 假设使用 CLI11 库
#include <tabulate/table.hpp>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <vector>

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
    table.add_row({
        "You Cooperate (C)",
        "R,R = " + std::to_string(R) + "," + std::to_string(R),
        "S,T = " + std::to_string(S) + "," + std::to_string(T)
        });

    // 第二行：You Defect
    table.add_row({
        "You Defect (D)",
        "T,S = " + std::to_string(T) + "," + std::to_string(S),
        "P,P = " + std::to_string(P) + "," + std::to_string(P)
        });

    // 格式化表格
    table.format()
        .border_color(tabulate::Color::blue)
        .font_align(tabulate::FontAlign::center);
        //.corner_corner('┼')
        //.corner_top_left('┌')
        //.corner_top_right('┐')
        //.corner_bottom_left('└')
        //.corner_bottom_right('┘')
        //.border_top('=')
        //.border_bottom('=')
        //.border_left('|')
        //.border_right('|');

    std::cout << table << std::endl;
    std::cout << "\nWhere:\n";
    std::cout << "  T (Temptation) = " << T << "  - Temptation to defect against a cooperator\n";
    std::cout << "  R (Reward)     = " << R << "  - Reward for mutual cooperation\n";
    std::cout << "  P (Punishment) = " << P << "  - Punishment for mutual defection\n";
    std::cout << "  S (Sucker)     = " << S << "  - Payoff for being betrayed when cooperating\n";
    std::cout << "\n";
}


// Run the corresponding simulation (standard tournament or evolutionary simulation).
void SimulatorRunner::runSimulation() {
    std::cout << "\n--- Tournament Start ---\n";
    if (config_.evolve) {
        // Note: Evolutionary logic has not been implemented in GameEngine yet.
        // This is just a placeholder indicating where the call should go.
        std::cout << "Evolution mode selected (feature not yet implemented).\n";
        // results_ = simulator_.runEvolution(...);
    }
    else {
        results_ = simulator_.runTournament(strategies_, config_.rounds, config_.repeats);
    }
}


// Print the final results.
void SimulatorRunner::printResults() const {
    std::cout << "\n=================================================\n";
    std::cout << "--- Tournament Results (Average Score per Strategy) ---\n";
    std::cout << "=================================================\n";

    // Create a sorted list of results
    std::vector<std::pair<std::string, double>> sorted_results(results_.begin(), results_.end());
    std::sort(sorted_results.begin(), sorted_results.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    int rank = 1;
    for (const auto& [name, score] : sorted_results) {
        std::cout << rank++ << ". " << std::setw(25) << std::left << name << ": "
            << std::fixed << std::setprecision(4) << score << "\n";
    }
    std::cout << "\n--- Simulation Complete ---\n";
}

// 新增：打印策略分析
void SimulatorRunner::printAnalysis() const {
    std::cout << "\n=================================================\n";
    std::cout << "--- 策略分析 (Strategy Analysis) ---\n";
    std::cout << "=================================================\n\n";

    std::cout << "在无噪声环境下的策略特点:\n\n";

    std::cout << "【合作型策略】\n\n";
    
    std::cout << "1. ALLC (Always Cooperate - 总是合作):\n";
    std::cout << "   - 策略描述: 无条件合作\n";
    std::cout << "   - 特点: 对合作型策略效果好，但容易被背叛型策略利用\n";
    std::cout << "   - 预期表现: 与TFT/GRIM/PAVLOV对战时得高分，遇到ALLD/PROBER时得最低分\n\n";

    std::cout << "2. TFT (Tit-For-Tat - 针锋相对):\n";
    std::cout << "   - 策略描述: 首轮合作，之后模仿对手上一轮行为\n";
    std::cout << "   - 特点: 善良、可激怒、宽容、清晰\n";
    std::cout << "   - 预期表现: 在无噪声环境下表现优秀，能与合作型策略互惠\n";
    std::cout << "   - 抗剥削性: 中等（能惩罚PROBER，但初期会被利用一次）\n\n";

    std::cout << "3. GRIM (Grim Trigger - 冷酷触发):\n";
    std::cout << "   - 策略描述: 开始合作，一旦对方背叛则永远背叛\n";
    std::cout << "   - 特点: 不可宽恕，严厉惩罚背叛者\n";
    std::cout << "   - 预期表现: 与合作型策略表现好，但对偶然错误不宽容\n";
    std::cout << "   - 抗剥削性: 高（PROBER首次背叛后立即永久背叛）\n\n";

    std::cout << "4. PAVLOV (Win-Stay-Lose-Shift):\n";
    std::cout << "   - 策略描述: 如果上轮结果好就保持，结果不好就改变\n";
    std::cout << "   - 特点: 基于结果调整，能从错误中恢复\n";
    std::cout << "   - 预期表现: 灵活应对，在某些情况下优于TFT\n";
    std::cout << "   - 抗剥削性: 中等（面对PROBER会在探测期调整）\n\n";

    std::cout << "5. CTFT (Contrite Tit-For-Tat - 愧疚针锋相对):\n";
    std::cout << "   - 策略描述: 首轮合作，如果背叛则在之后的轮次中更加宽容\n";
    std::cout << "   - 特点: 宽容、反应灵敏\n";
    std::cout << "   - 预期表现: 在偶尔错误的情况下表现优于TFT\n";
    std::cout << "   - 抗剥削性: 中等（宽容性可能被利用）\n\n";

    std::cout << "【剥削型策略】\n\n";
    
    std::cout << "6. ALLD (Always Defect - 总是背叛):\n";
    std::cout << "   - 策略描述: 无条件背叛\n";
    std::cout << "   - 特点: 短期利益最大化，但长期合作收益差\n";
    std::cout << "   - 预期表现: 对ALLC能获得最高分(T)，但与其他策略长期对战只能获得P\n";
    std::cout << "   - 剥削能力: 高（对无条件合作者），但无法建立互惠关系\n\n";

    std::cout << "7. PROBER (探测者 - 试探性剥削):\n";
    std::cout << "   - 策略描述: 前3轮采用D-C-C模式探测对手，根据反应决定策略\n";
    std::cout << "   - 特点: 智能剥削，区分对待不同对手\n";
    std::cout << "   - 行为逻辑:\n";
    std::cout << "     * 如果对手在探测期（第2、3轮）都合作 → 永远背叛（剥削模式）\n";
    std::cout << "     * 如果对手有报复行为 → 切换到TFT模式（防御模式）\n";
    std::cout << "   - 预期表现:\n";
    std::cout << "     * vs ALLC: 高分（成功剥削）\n";
    std::cout << "     * vs TFT: 中等（探测失败，转为互惠）\n";
    std::cout << "     * vs GRIM: 低分（首次背叛导致永久对抗）\n";
    std::cout << "   - 剥削能力: 智能型（选择性剥削弱者，与强者合作）\n\n";

    std::cout << "【剥削性分析】\n\n";
    
    std::cout << "抗剥削能力排名（从强到弱）:\n";
    std::cout << "  1. GRIM     - 零容忍策略，立即永久报复\n";
    std::cout << "  2. TFT      - 快速报复，但给予改过机会\n";
    std::cout << "  3. PAVLOV   - 基于收益调整，有一定抗性\n";
    std::cout << "  4. CTFT     - 过度宽容可能被利用\n";
    std::cout << "  5. ALLC     - 完全无抵抗，最易被剥削\n\n";

    std::cout << "PROBER vs 各策略预期:\n";
    std::cout << "  • PROBER vs ALLC:  完全剥削（探测成功→永久背叛）\n";
    std::cout << "  • PROBER vs TFT:   部分损失（首轮被利用，之后互惠）\n";
    std::cout << "  • PROBER vs GRIM:  双输局面（触发永久对抗）\n";
    std::cout << "  • PROBER vs PAVLOV: 动态博弈（取决于探测反应）\n";
    std::cout << "  • PROBER vs ALLD:  互相背叛（双方得P）\n\n";

    std::cout << "理论预测（无噪声环境）:\n";
    std::cout << "  在混合种群中:\n";
    std::cout << "  - PROBER能剥削ALLC，与TFT/PAVLOV达成部分合作\n";
    std::cout << "  - GRIM能有效威慑PROBER，但对噪声敏感\n";
    std::cout << "  - ALLD在所有匹配中得分稳定但较低（多为P）\n";
    std::cout << "  - 合作型策略互相对战时得分最高（R）\n\n";
    
    std::cout << "噪声环境下的影响:\n";
    std::cout << "  - GRIM: 严重崩溃（无法从偶然错误恢复）\n";
    std::cout << "  - TFT: 中等影响（可能陷入报复循环）\n";
    std::cout << "  - PROBER: 探测准确性下降（噪声干扰判断）\n";
    std::cout << "  - CTFT/PAVLOV: 表现更稳健（纠错能力强）\n";
    std::cout << "  - ALLD: 不受影响（无条件背叛）\n";
    std::cout << "\n=================================================\n\n";
}

// 运行噪声扫描
void SimulatorRunner::runNoiseSweep() {
    std::vector<double> noise_levels;
    for (double eps = config_.noise_min; eps <= config_.noise_max + 0.001; eps += config_.noise_step) {
        noise_levels.push_back(eps);
    }

    auto results = simulator_.runNoiseSweep(strategies_, config_.rounds, config_.repeats, noise_levels);
    
    // 打印汇总表格
    Simulator::printNoiseSweepTable(results);
    
    // 打印分析
    Simulator::analyzeNoiseImpact(results);
}

// Handle all command-line parsing and return a Config structure.
Config SimulatorRunner::parseArguments(int argc, char** argv) {
    CLI::App app{ "Iterated Prisoner's Dilemma Simulator" };
    Config config;

    // Set up CLI options and bind them to members of the config structure.
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

    // Noise sweep options
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
