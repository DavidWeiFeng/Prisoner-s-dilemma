#ifndef CONFIGIO_H
#define CONFIGIO_H

#include "Config.h"
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>

/**
 * @class ConfigIO
 * @brief Handles saving and loading configuration to/from JSON files
 */
class ConfigIO {
public:
    // Save configuration to JSON file
    static void saveConfig(const Config& config, const std::string& filename);
    
    // Load configuration from JSON file
    static Config loadConfig(const std::string& filename);
    
private:
    // Helper to escape JSON strings
    static std::string escapeJson(const std::string& str);
    
    // Helper to parse JSON string value
    static std::string parseJsonString(const std::string& json, const std::string& key);
    
    // Helper to parse JSON integer value
    static int parseJsonInt(const std::string& json, const std::string& key);
    
    // Helper to parse JSON double value
    static double parseJsonDouble(const std::string& json, const std::string& key);
    
    // Helper to parse JSON boolean value
    static bool parseJsonBool(const std::string& json, const std::string& key);
    
    // Helper to parse JSON array of doubles
    static std::vector<double> parseJsonDoubleArray(const std::string& json, const std::string& key);
    
    // Helper to parse JSON array of strings
    static std::vector<std::string> parseJsonStringArray(const std::string& json, const std::string& key);
};

#endif // CONFIGIO_H
