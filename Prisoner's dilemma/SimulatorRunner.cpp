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
    printPayoffMatrix();  // 新增：打印收益矩阵
    runSimulation();
    printResults();
    printAnalysis();      // 新增：打印策略分析
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
    std::cout << "\n=================================================\n";
    std::cout << "    囚徒困境基线循环赛 (Round-Robin Tournament)\n";
    std::cout << "=================================================\n\n";
    std::cout << "--- 模拟配置 ---\n"
        << "每场比赛的回合数: " << config_.rounds << "\n"
        << "每场比赛的重复次数: " << config_.repeats << "\n"
        << "噪声（Epsilon）: " << config_.epsilon << " (无噪声环境)\n"
        << "随机种子: " << config_.seed << "\n"
        << "收益 (T,R,P,S): " << std::fixed << std::setprecision(1)
        << config_.payoffs[0] << "," << config_.payoffs[1] << ","
        << config_.payoffs[2] << "," << config_.payoffs[3] << "\n"
        << "参赛策略 (" << strategies_.size() << "个): ";
    for (const auto& s : strategies_) {
        std::cout << s->getName() << " ";
    }
    std::cout << "\n";

    if (config_.evolve) {
        std::cout << "\n--- 进化参数 ---\n"
            << "种群数量: " << config_.population << "\n"
            << "进化代数: " << config_.generations << "\n"
            << "突变率: " << std::fixed << std::setprecision(2) << config_.mutation << "\n";
    }
}

// 新增：打印收益矩阵
void SimulatorRunner::printPayoffMatrix() const {
    double T = config_.payoffs[0];
    double R = config_.payoffs[1];
    double P = config_.payoffs[2];
    double S = config_.payoffs[3];

    std::cout << "\n--- 收益矩阵 (Payoff Matrix) ---\n";
    std::cout << "基于经典囚徒困境参数: T > R > P > S 且 2R > T + S\n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "\n";
    std::cout << "                    对手合作(C)    对手背叛(D)\n";
    std::cout << "              +------------------+------------------+\n";
    std::cout << "  自己合作(C) |   R,R = " << std::setw(4) << R << "," << std::setw(4) << R
        << "  |   S,T = " << std::setw(4) << S << "," << std::setw(4) << T << "  |\n";
    std::cout << "              +------------------+------------------+\n";
    std::cout << "  自己背叛(D) |   T,S = " << std::setw(4) << T << "," << std::setw(4) << S
        << "  |   P,P = " << std::setw(4) << P << "," << std::setw(4) << P << "  |\n";
    std::cout << "              +------------------+------------------+\n";
    std::cout << "\n其中:\n";
    std::cout << "  T (Temptation) = " << T << "  - 背叛合作者的诱惑\n";
    std::cout << "  R (Reward)     = " << R << "  - 互相合作的奖励\n";
    std::cout << "  P (Punishment) = " << P << "  - 互相背叛的惩罚\n";
    std::cout << "  S (Sucker)     = " << S << "  - 被背叛的傻瓜报酬\n";
    std::cout << "\n";
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
    std::cout << "\n=================================================\n";
    std::cout << "--- 锦标赛结果 (各策略平均分) ---\n";
    std::cout << "=================================================\n";

    // 创建一个排序的结果列表
    std::vector<std::pair<std::string, double>> sorted_results(results_.begin(), results_.end());
    std::sort(sorted_results.begin(), sorted_results.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    int rank = 1;
    for (const auto& [name, score] : sorted_results) {
        std::cout << rank++ << ". " << std::setw(25) << std::left << name << ": "
            << std::fixed << std::setprecision(4) << score << "\n";
    }
    std::cout << "\n--- 模拟结束 ---\n";
}

// 新增：打印策略分析
void SimulatorRunner::printAnalysis() const {
    std::cout << "\n=================================================\n";
    std::cout << "--- 策略分析 (Strategy Analysis) ---\n";
    std::cout << "=================================================\n\n";

    std::cout << "在无噪声环境下的策略特点:\n\n";

    std::cout << "1. ALLC (Always Cooperate - 总是合作):\n";
    std::cout << "   - 策略描述: 无条件合作\n";
    std::cout << "   - 特点: 对合作型策略效果好，但容易被背叛型策略利用\n";
    std::cout << "   - 预期表现: 与TFT/GRIM/PAVLOV对战时得高分，遇到ALLD时得最低分\n\n";

    std::cout << "2. ALLD (Always Defect - 总是背叛):\n";
    std::cout << "   - 策略描述: 无条件背叛\n";
    std::cout << "   - 特点: 短期利益最大化，但长期合作收益差\n";
    std::cout << "   - 预期表现: 对ALLC能获得高分，但与其他策略长期对战得分低\n\n";

    std::cout << "3. TFT (Tit-For-Tat - 针锋相对):\n";
    std::cout << "   - 策略描述: 首轮合作，之后模仿对手上一轮行为\n";
    std::cout << "   - 特点: 善良、可激怒、宽容、清晰\n";
    std::cout << "   - 预期表现: 在无噪声环境下表现优秀，能与合作型策略互惠\n\n";

    std::cout << "4. GRIM (Grim Trigger - 冷酷触发):\n";
    std::cout << "   - 策略描述: 开始合作，一旦对方背叛则永远背叛\n";
    std::cout << "   - 特点: 不可宽恕，严厉惩罚背叛者\n";
    std::cout << "   - 预期表现: 与合作型策略表现好，但对偶然错误不宽容\n\n";

    std::cout << "5. PAVLOV (Win-Stay-Lose-Shift):\n";
    std::cout << "   - 策略描述: 如果上轮结果好就保持，结果不好就改变\n";
    std::cout << "   - 特点: 基于结果调整，能从错误中恢复\n";
    std::cout << "   - 预期表现: 灵活应对，在某些情况下优于TFT\n\n";

    std::cout << "理论预测（无噪声环境）:\n";
    std::cout << "- 最佳策略: TFT、GRIM、PAVLOV（互相合作获得高分）\n";
    std::cout << "- 中等策略: ALLC（被ALLD利用导致总分下降）\n";
    std::cout << "- 最差策略: ALLD（无法建立合作关系）\n";
    std::cout << "\n=================================================\n\n";
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
        throw std::runtime_error("命令行参数解析错误。请检查您的输入。错误: " + std::string(e.what()));
    }

    return config;
}