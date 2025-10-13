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

/**
 * @class SimulatorRunner
 * @brief 协调主要的应用逻辑。
 *
 * 该类负责根据提供的配置来设置策略、打印配置信息、
 * 运行模拟并显示结果。它封装了程序的核心工作流程。
 */
class SimulatorRunner {
public:
    explicit SimulatorRunner(const Config& config);
    void run();
    static Config parseArguments(int argc, char** argv);

private:
    Config config_;
    std::vector<std::unique_ptr<Strategy>> strategies_;
    Simulator simulator_;

    std::map<double, std::map<std::string, ScoreStats>> results_;//噪声级别 -> 策略 -> 统计信息

    void setupStrategies();
    void printConfiguration() const;
    void printPayoffMatrix() const;

    void runSimulation();
	void runExploiter();
    void runNoiseSweep();

    void printResults() const;
    void printAnalysisQ1() const;
    void printExploiterResults(
        const std::string& exploiter_name,
		const std::map<std::string, std::pair<ScoreStats, ScoreStats>>& results) const;
    // 根据策略名称创建策略实例的工厂函数。
    static std::unique_ptr<Strategy> createStrategy(const std::string& name);

};

#endif // SIMULATORRUNNER_H