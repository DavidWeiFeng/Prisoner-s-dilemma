#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>

#include "SimulatorRunner.h"
#include "Config.h"

int main(int argc, char** argv) {
    try {
        // 1. 解析命令行参数，生成一个配置对象。
        Config config = SimulatorRunner::parseArguments(argc, argv);

        // 2. 使用该配置创建并运行应用。
        SimulatorRunner runner(config);
        runner.run();

    }
    catch (const std::exception& e) {
        // 捕获任何标准异常 (例如，来自命令行解析或文件错误)
        // 并友好地报告它们。
        std::cerr << "错误: " << e.what() << std::endl;
        return 1; // 返回 1 表示执行失败
    }

    return 0; // 返回 0 表示执行成功
}

