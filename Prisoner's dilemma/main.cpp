#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>
#include "SimulatorRunner.h"
#include "Config.h"
int main(int argc, char** argv) {
    try {
        // 1. Parse command-line arguments to generate a configuration object.
        Config config = SimulatorRunner::parseArguments(argc, argv);

        // 2. Create and run the application using this configuration.
        SimulatorRunner runner(config);
        runner.run();

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1; 
    }

    return 0; 
}

