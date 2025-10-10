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
    int rounds = 150;
    int repeats = 50;
    double epsilon = 0.0;
    int seed = 42;
    std::vector<double> payoffs = { 5.0, 3.0, 1.0, 0.0 }; // T, R, P, S
    std::vector<std::string> strategy_names = { "AllCooperate", "AllDefect","TitForTat","GrimTrigger","PAVLOV","CTFT","PROBER"};

    // 输入/输出参数
    std::string format = "text";
    std::string save_file;
    std::string load_file;

    // 进化参数
    bool evolve = false;
    int population = 200;
    int generations = 200;
    double mutation = 0.02;
    
    // 噪声扫描参数
    bool noise_sweep = true;       // 默认启用噪声扫描
    double noise_min = 0.0;        // 最小噪声水平
    double noise_max = 0.2;        // 最大噪声水平
    double noise_step = 0.05;      // 噪声步长
};

#endif // CONFIG_H
