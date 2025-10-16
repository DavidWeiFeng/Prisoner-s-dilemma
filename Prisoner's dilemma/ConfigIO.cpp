#include "ConfigIO.h"
#include <iostream>
#include <algorithm>
#include <cctype>

std::string ConfigIO::escapeJson(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\t') result += "\\t";
        else result += c;
    }
    return result;
}

void ConfigIO::saveConfig(const Config& config, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + filename);
    }
    
    file << "{\n";
    file << "  \"rounds\": " << config.rounds << ",\n";
    file << "  \"repeats\": " << config.repeats << ",\n";
    file << "  \"epsilon\": " << config.epsilon << ",\n";
    file << "  \"seed\": " << config.seed << ",\n";
    
    // Payoffs array
    file << "  \"payoffs\": [";
    for (size_t i = 0; i < config.payoffs.size(); ++i) {
        file << config.payoffs[i];
        if (i < config.payoffs.size() - 1) file << ", ";
    }
    file << "],\n";
    
    // Strategy names array
    file << "  \"strategy_names\": [";
    for (size_t i = 0; i < config.strategy_names.size(); ++i) {
        file << "\"" << escapeJson(config.strategy_names[i]) << "\"";
        if (i < config.strategy_names.size() - 1) file << ", ";
    }
    file << "],\n";
    
    file << "  \"format\": \"" << escapeJson(config.format) << "\",\n";
    file << "  \"save_file\": \"" << escapeJson(config.save_file) << "\",\n";
    file << "  \"load_file\": \"" << escapeJson(config.load_file) << "\",\n";
    
    // Q2: Noise sweep parameters
    file << "  \"noise_sweep\": " << (config.noise_sweep ? "true" : "false") << ",\n";
    file << "  \"epsilon_values\": [";
    for (size_t i = 0; i < config.epsilon_values.size(); ++i) {
        file << config.epsilon_values[i];
        if (i < config.epsilon_values.size() - 1) file << ", ";
    }
    file << "],\n";
    
    // Q3: Exploiter test parameters
    file << "  \"show_exploiter\": " << (config.show_exploiter ? "true" : "false") << ",\n";
    file << "  \"analyze_mixed\": " << (config.analyze_mixed ? "true" : "false") << ",\n";
    file << "  \"exploiter_noise_compare\": " << (config.exploiter_noise_compare ? "true" : "false") << ",\n";
    
    // Q4: Evolution parameters
    file << "  \"evolve\": " << (config.evolve ? "true" : "false") << ",\n";
    file << "  \"generations\": " << config.generations << ",\n";
    
    // Q5: SCB parameters
    file << "  \"enable_scb\": " << (config.enable_scb ? "true" : "false") << ",\n";
    file << "  \"scb_cost_factor\": " << config.scb_cost_factor << ",\n";
    file << "  \"scb_compare\": " << (config.scb_compare ? "true" : "false") << "\n";
    
    file << "}\n";
    
    file.close();
    std::cout << "Configuration saved to: " << filename << std::endl;
}

std::string ConfigIO::parseJsonString(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return "";
    
    size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) return "";
    
    size_t startQuote = json.find('"', colonPos);
    if (startQuote == std::string::npos) return "";
    
    size_t endQuote = json.find('"', startQuote + 1);
    if (endQuote == std::string::npos) return "";
    
    return json.substr(startQuote + 1, endQuote - startQuote - 1);
}

int ConfigIO::parseJsonInt(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) throw std::runtime_error("Key not found: " + key);
    
    size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) throw std::runtime_error("Invalid JSON format");
    
    size_t numStart = colonPos + 1;
    while (numStart < json.size() && std::isspace(json[numStart])) ++numStart;
    
    size_t numEnd = numStart;
    while (numEnd < json.size() && (std::isdigit(json[numEnd]) || json[numEnd] == '-')) ++numEnd;
    
    std::string numStr = json.substr(numStart, numEnd - numStart);
    return std::stoi(numStr);
}

double ConfigIO::parseJsonDouble(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) throw std::runtime_error("Key not found: " + key);
    
    size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) throw std::runtime_error("Invalid JSON format");
    
    size_t numStart = colonPos + 1;
    while (numStart < json.size() && std::isspace(json[numStart])) ++numStart;
    
    size_t numEnd = numStart;
    while (numEnd < json.size() && (std::isdigit(json[numEnd]) || json[numEnd] == '.' || json[numEnd] == '-' || json[numEnd] == 'e' || json[numEnd] == 'E' || json[numEnd] == '+')) ++numEnd;
    
    std::string numStr = json.substr(numStart, numEnd - numStart);
    return std::stod(numStr);
}

bool ConfigIO::parseJsonBool(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return false;
    
    size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) return false;
    
    size_t truePos = json.find("true", colonPos);
    size_t falsePos = json.find("false", colonPos);
    
    if (truePos != std::string::npos && (falsePos == std::string::npos || truePos < falsePos)) {
        // Check that 'true' comes before next comma or closing brace
        size_t nextDelim = json.find_first_of(",}", colonPos);
        if (truePos < nextDelim) return true;
    }
    
    return false;
}

std::vector<double> ConfigIO::parseJsonDoubleArray(const std::string& json, const std::string& key) {
    std::vector<double> result;
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return result;
    
    size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) return result;
    
    size_t arrayStart = json.find('[', colonPos);
    if (arrayStart == std::string::npos) return result;
    
    size_t arrayEnd = json.find(']', arrayStart);
    if (arrayEnd == std::string::npos) return result;
    
    std::string arrayContent = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
    std::istringstream iss(arrayContent);
    std::string token;
    
    while (std::getline(iss, token, ',')) {
        // Trim whitespace
        token.erase(0, token.find_first_not_of(" \t\n\r"));
        token.erase(token.find_last_not_of(" \t\n\r") + 1);
        
        if (!token.empty()) {
            result.push_back(std::stod(token));
        }
    }
    
    return result;
}

std::vector<std::string> ConfigIO::parseJsonStringArray(const std::string& json, const std::string& key) {
    std::vector<std::string> result;
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return result;
    
    size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) return result;
    
    size_t arrayStart = json.find('[', colonPos);
    if (arrayStart == std::string::npos) return result;
    
    size_t arrayEnd = json.find(']', arrayStart);
    if (arrayEnd == std::string::npos) return result;
    
    std::string arrayContent = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
    
    // Parse quoted strings
    size_t pos = 0;
    while (pos < arrayContent.size()) {
        size_t quoteStart = arrayContent.find('"', pos);
        if (quoteStart == std::string::npos) break;
        
        size_t quoteEnd = arrayContent.find('"', quoteStart + 1);
        if (quoteEnd == std::string::npos) break;
        
        result.push_back(arrayContent.substr(quoteStart + 1, quoteEnd - quoteStart - 1));
        pos = quoteEnd + 1;
    }
    
    return result;
}

Config ConfigIO::loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for reading: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json = buffer.str();
    file.close();
    
    Config config;
    
    // Parse basic parameters
    try {
        config.rounds = parseJsonInt(json, "rounds");
        config.repeats = parseJsonInt(json, "repeats");
        config.epsilon = parseJsonDouble(json, "epsilon");
        config.seed = parseJsonInt(json, "seed");
        
        config.payoffs = parseJsonDoubleArray(json, "payoffs");
        config.strategy_names = parseJsonStringArray(json, "strategy_names");
        
        config.format = parseJsonString(json, "format");
        config.save_file = parseJsonString(json, "save_file");
        config.load_file = parseJsonString(json, "load_file");
        
        config.noise_sweep = parseJsonBool(json, "noise_sweep");
        config.epsilon_values = parseJsonDoubleArray(json, "epsilon_values");
        
        config.show_exploiter = parseJsonBool(json, "show_exploiter");
        config.analyze_mixed = parseJsonBool(json, "analyze_mixed");
        config.exploiter_noise_compare = parseJsonBool(json, "exploiter_noise_compare");
        
        config.evolve = parseJsonBool(json, "evolve");
        config.generations = parseJsonInt(json, "generations");
        
        config.enable_scb = parseJsonBool(json, "enable_scb");
        config.scb_cost_factor = parseJsonDouble(json, "scb_cost_factor");
        config.scb_compare = parseJsonBool(json, "scb_compare");
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error parsing JSON config file: " + std::string(e.what()));
    }
    
    std::cout << "Configuration loaded from: " << filename << std::endl;
    return config;
}
