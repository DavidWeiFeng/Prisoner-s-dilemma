#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <string>

/**
 * @struct Config
 * @brief 将所有模拟参数保存在一个可传递的对象中。
 *
 * 这个结构体整合了所有来自命令行的设置，提供了默认值，
 * 并使得在整个应用程序中传递参数变得非常整洁。
 */
struct Config {
    // 锦标赛参数
    int rounds = 10;
    int repeats = 2;
    double epsilon = 0;
    int seed = 42;
    std::vector<double> payoffs = { 5.0, 3.0, 1.0, 0.0 }; // T, R, P, S

    
    //std::vector<std::string> strategy_names = { "AllCooperate", "AllDefect","TitForTat","GrimTrigger","PAVLOV"};
    std::vector<std::string> strategy_names = { "AllCooperate", "AllDefect","TitForTat","GrimTrigger","PAVLOV","ContriteTitForTat","PROBER" };
     //std::vector<std::string> strategy_names = { "AllDefect","AllCooperate","TitForTat","ContriteTitForTat","PAVLOV"};
    //std::vector<std::string> strategy_names = { "PROBER","AllCooperate","TitForTat" };
    //std::vector<std::string> strategy_names = { "TitForTat","GrimTrigger","PAVLOV","ContriteTitForTat" };

    std::string format = "csv";
    std::string save_file;
    std::string load_file;
    
    // Q2:噪声扫描参数
    bool noise_sweep = false;           // 是否启用噪声扫描模式
    std::vector<double> epsilon_values = {0.0, 0.05, 0.1, 0.15, 0.2}; // 噪声水平扫描值
    
    // Q3: 剥削者测试参数
    bool show_exploiter = false;       // 是否展示剥削者 vs 对手的详细对战
    bool analyze_mixed = true;        // 是否分析剥削者在混合人群中的表现
	//Q4:进化模拟参数
    bool evolve = false;
    int generations = 50;
    // Q5:SCB (Strategic Complexity Budget) 参数
    bool enable_scb = false;           // 是否启用战略复杂度预算
    double scb_cost_factor = 0.1;      // 每单位复杂度每轮的成本系数
};

#endif // CONFIG_H
