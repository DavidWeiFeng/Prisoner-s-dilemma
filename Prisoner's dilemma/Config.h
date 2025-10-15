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
    double epsilon = 0.05;
    int seed = 42;
    std::vector<double> payoffs = { 5.0, 3.0, 1.0, 0.0 }; // T, R, P, S

    
    //std::vector<std::string> strategy_names = { "AllCooperate", "AllDefect","TitForTat","GrimTrigger","PAVLOV"};
    //std::vector<std::string> strategy_names = { "AllCooperate", "AllDefect","TitForTat","GrimTrigger","PAVLOV","CTFT","PROBER" };

     //std::vector<std::string> strategy_names = { "AllDefect","AllCooperate","TitForTat","ContriteTitForTat","PAVLOV"};
     std::vector<std::string> strategy_names = { "PROBER","AllCooperate","TitForTat" };


    std::string format = "text";
    std::string save_file;
    std::string load_file;

    // 进化参数
    bool evolve = true;
    int generations = 50;
	bool  exploiters = false;      // 是否包含剥削者策略

};

#endif // CONFIG_H
