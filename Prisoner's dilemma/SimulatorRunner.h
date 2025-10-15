#ifndef SIMULATORRUNNER_H
#define SIMULATORRUNNER_H

#include <vector>
#include <memory>
#include <map>
#include <string>
#include <algorithm>
#include "Config.h"
#include "Strategy.h"
#include "Simulator.h"
#include "ResultsPrinter.h"

/**
 * @class SimulatorRunner
 * @brief 协调主要的应用逻辑。
 *
 * 该类负责根据提供的配置来设置策略、运行模拟并协调输出。
 * 所有打印功能已移至 ResultsPrinter 类。
 */
class SimulatorRunner {
public:
    // 使用配置对象来初始化应用状态。
    explicit SimulatorRunner(const Config& config);

    // 执行模拟的主入口点。
    void run();

    // 用于处理命令行参数解析的静态方法。
    static Config parseArguments(int argc, char** argv);

private:

    // 根据策略名称创建策略实例的工厂函数。
    static std::unique_ptr<Strategy> createStrategy(const std::string& name);
    Config config_;
    std::vector<std::unique_ptr<Strategy>> strategies_;
    Simulator simulator_;
    std::map<std::string, ScoreStats> results_; // 用于存储模拟结果（包括置信区间）
    ResultsPrinter printer_;


    // 根据配置中的名称初始化策略对象向量。
    void setupStrategies();

    // 执行主要的锦标赛或进化模拟。
    void runSimulation();
    void runExploiter();

    // 新增：运行进化模拟
    void runEvolution();
    std::vector<std::map<std::string, double>> runSingleEvolution(double noise, const std::string& label);
    std::map<std::string, double> calculateFitness(const std::map<std::string, double>& populations, int rounds, int repeats);
    
    // 新增：运行噪声扫描
    void runNoiseSweep();
    std::map<double, std::map<std::string, ScoreStats>> executeNoiseSweep(const std::vector<double>& epsilon_values);

    // Q3: 运行剥削者详细对战
    void runShowExploiter();
    
    // Q3: 运行混合人群分析
    void runMixedPopulationAnalysis();

    double playMultipleGames(
        const std::unique_ptr<Strategy>& strat_i,
        const std::unique_ptr<Strategy>& strat_j,
        int rounds, int repeats);

    void updatePopulations(
        std::map<std::string, double>& populations,
		const std::map<std::string, double>& fitness);

    // SCB: 运行带有 SCB 对比的锦标赛
    void runSCBComparison();
    void printExploiterMatchTable(
        const std::string& exploiter_name,
        const std::map<std::string, std::pair<double, double>>& matchAverages) const;

    void printExploiterResults(
        const std::string& exploiter_name,
		const std::map<std::string, std::pair<ScoreStats, ScoreStats>>& results) const;
};

#endif // SIMULATORRUNNER_H