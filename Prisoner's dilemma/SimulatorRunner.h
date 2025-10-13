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
    // 使用配置对象来初始化应用状态。
    explicit SimulatorRunner(const Config& config);

    // 执行模拟的主入口点。
    void run();

    // 用于处理命令行参数解析的静态方法。
    static Config parseArguments(int argc, char** argv);

private:
    // 根据配置中的名称初始化策略对象向量。
    void setupStrategies();

    // 将模拟和进化参数打印到控制台。
    void printConfiguration() const;

    // 打印收益矩阵
    void printPayoffMatrix() const;

    // 执行主要的锦标赛或进化模拟。
    void runSimulation();

    // 运行噪声扫描实验
    void runNoiseSweep();

    // 以用户指定的格式打印最终结果。
    void printResults() const;

    // 打印策略分析
    void printAnalysisQ1() const;

    // 根据策略名称创建策略实例的工厂函数。
    static std::unique_ptr<Strategy> createStrategy(const std::string& name);

    Config config_;
    std::vector<std::unique_ptr<Strategy>> strategies_;
    Simulator simulator_;
    std::map<std::string, ScoreStats> results_; // 用于存储模拟结果（包括置信区间）
};

#endif // SIMULATORRUNNER_H