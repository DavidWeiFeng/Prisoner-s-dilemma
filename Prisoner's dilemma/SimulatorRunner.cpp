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
        printResults();
    }
    else if (config_.evolve) {
        // 运行进化模拟
        runEvolution();
    }
    else {
        runSimulation();
        printResults();
        //printAnalysisQ1();
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
	Strategy::setNoise(config_.epsilon); // 设置全局噪声水平
    for (const auto& name : config_.strategy_names) {
        auto strat = createStrategy(name);
        if (!strat) {
            throw std::runtime_error("发现未知策略: " + name);
        }
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
        table.add_row({ "Generations", std::to_string(config_.generations) });
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

    //the fitst one is exploiter
    const auto& exploiter = strategies_[0];
    std::string exploiter_name = exploiter->getName();
    std::cout << "Exploiter: " << exploiter_name << "\n";
    std::cout << "Victims: ";
    for (size_t i = 1; i < strategies_.size(); ++i) {
        std::cout << strategies_[i]->getName();
        if (i < strategies_.size() - 1) std::cout << ", ";
    }
    std::cout << "\n\n";

    // 存储所有策略的分数
    std::map<std::string, std::vector<double>> allScores;
    allScores[exploiter_name] = std::vector<double>();

    std::map<std::string, std::pair<double, double>> matchAverages; // victim_name -> (exploiter_avg, victim_avg)

    // 初始化每个受害者的分数向量
    for (size_t i = 1; i < strategies_.size(); ++i) {
        allScores[strategies_[i]->getName()] = std::vector<double>();
    }

    for (size_t i = 1; i < strategies_.size(); ++i) {
        const auto& victim = strategies_[i];
        std::string victim_name = victim->getName();
        std::vector<double> exploiter_scores_this_match;
        std::vector<double> victim_scores_this_match;

        // repeats competition
        for (int r = 0; r < config_.repeats; ++r) {
            // reset
            exploiter->reset();
            victim->reset();

            // 运行单场游戏
            ScorePair scores = simulator_.runGame(exploiter, victim, config_.rounds);

            // 记录分数
            allScores[exploiter_name].push_back(scores.first);
            allScores[victim_name].push_back(scores.second);

            exploiter_scores_this_match.push_back(scores.first);
            victim_scores_this_match.push_back(scores.second);

        }
        double exploiter_avg = std::accumulate(exploiter_scores_this_match.begin(),exploiter_scores_this_match.end(), 0.0) / config_.repeats;
        double victim_avg = std::accumulate(victim_scores_this_match.begin(),victim_scores_this_match.end(), 0.0) / config_.repeats;
        matchAverages[victim_name] = { exploiter_avg, victim_avg };
    }
	//print averge table
    printExploiterMatchTable(exploiter_name, matchAverages);

    results_.clear();
    for (const auto& [name, scores] : allScores) {
        results_[name] = simulator_.calculateStats(scores);
    }
    std::cout << "\n--- All exploiter matches completed ---\n";
}

void SimulatorRunner::runEvolution() {
    std::cout << "\n=================================================\n";
    std::cout << "    Evolutionary Tournament\n";
    std::cout << "=================================================\n\n";

    auto history_noisefree = runSingleEvolution(0.0, "Noise-Free, epsilon=0.0");
    auto history_noisy = runSingleEvolution(config_.epsilon, "Noisy, epsilon=" + std::to_string(config_.epsilon));
}

std::vector<std::map<std::string, double>>
SimulatorRunner::runSingleEvolution(double noise, const std::string& label) {
    std::cout << "\n--- Running Evolution (" << label << ") ---\n";
	Strategy::setNoise(noise); // 设置噪声水平

	std::map<std::string, double> populations;
	double initial_fraction = 1.0 / strategies_.size();
    for (const auto& s : strategies_)
    {
        populations[s->getName()] = initial_fraction;
    }
	std::vector<std::map<std::string, double>> history;
    for (int gen = 0; gen < config_.generations; gen++)
    {
		history.push_back(populations);
        if (gen % 5 == 0)
        {
            printGeneration(gen, populations);
        }
        // 最后一代不需要更新
        if (gen == config_.generations) break;

        // 计算适应度并更新种群
        auto fitness = calculateFitness(populations, config_.rounds, config_.repeats);
        updatePopulations(populations, fitness);
    }
	return history;
}
std::map<std::string, double> SimulatorRunner::calculateFitness(const std::map<std::string, double>& populations,int rounds, int repeats) {

    std::map<std::string, double> fitness;

    // 每个策略与每个其他策略对战
    for (const auto& strat_i : strategies_) {
        std::string name_i = strat_i->getName();
        double pop_i = populations.at(name_i);

        if (pop_i < 1e-6) {
            fitness[name_i] = 0.0;  // 已灭绝的策略适应度为0
            continue;
        }

        double total_fitness = 0.0;

        for (const auto& strat_j : strategies_) {
            std::string name_j = strat_j->getName();
            double pop_j = populations.at(name_j);

            if (pop_j < 1e-6) continue;  // 跳过灭绝的对手

            // 计算对战这个策略的平均得分
            double avg_score = playMultipleGames(strat_i, strat_j, rounds, repeats);

            // 加权贡献到适应度
            total_fitness += avg_score * pop_j;
        }

        fitness[name_i] = total_fitness;
    }

    return fitness;
}

double SimulatorRunner::playMultipleGames(
    const std::unique_ptr<Strategy>& strat_i,
    const std::unique_ptr<Strategy>& strat_j,
    int rounds, int repeats) {

    double total_score = 0.0;
    bool is_self_play = (strat_i->getName() == strat_j->getName());

    for (int r = 0; r < repeats; ++r) {
        strat_i->reset();
        strat_j->reset();

        ScorePair scores;
        if (is_self_play) {
            auto clone = strat_i->clone();
            scores = simulator_.runGame(strat_i, clone, rounds);
        }
        else {
            scores = simulator_.runGame(strat_i, strat_j, rounds);
        }
        total_score += scores.first;
    }

    return total_score / repeats;
}
void SimulatorRunner::updatePopulations(
    std::map<std::string, double>& populations,
    const std::map<std::string, double>& fitness) {

    // 步骤1: 计算平均适应度
    double avg_fitness = 0.0;
    for (const auto& [name, pop] : populations) {
        avg_fitness += fitness.at(name) * pop;
    }

    // 避免除零错误
    if (avg_fitness < 1e-9) {
        std::cerr << "Warning: Average fitness is too low, skipping update.\n";
        return;
    }

    // 步骤2: 应用复制器动力学公式更新每个策略的比例
    // new_pop[i] = pop[i] * (fitness[i] / avg_fitness)
    std::map<std::string, double> new_populations;
    for (const auto& [name, pop] : populations) {
        new_populations[name] = pop * (fitness.at(name) / avg_fitness);
    }

    // 步骤3: 更新原始种群
    populations = new_populations;

    // 可选：验证总和是否为1（用于调试）
    double sum = 0.0;
    for (const auto& [name, pop] : populations) {
        sum += pop;
    }
    if (std::abs(sum - 1.0) > 1e-6) {
        std::cerr << "Warning: Population sum = " << sum << " (should be 1.0)\n";
    }
}


void SimulatorRunner::printGeneration(int gen, const std::map<std::string, double>& populations) {
    std::cout << "Generation " << gen << ": ";
    for (const auto& [name, pop] : populations) {
        std::cout << name << "=" << std::fixed << std::setprecision(3) << pop << " ";
    }
    std::cout << "\n";
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
    app.add_option("--generations", config.generations, "Number of generations for the evolutionary simulation.");
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
    table.add_row({ "Rank", "Strategy", "Mean", "95% CI Lower", "95% CI Upper", "Std Dev" });

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

void SimulatorRunner::printExploiterMatchTable(
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

    // 为表头设置样式
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
            format_double(exploiter_score),
            format_double(victim_score),
            format_double(difference)
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
            format_double(avg_exploiter),
            format_double(avg_victim),
            format_double(avg_difference)
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