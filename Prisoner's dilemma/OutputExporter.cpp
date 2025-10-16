#include "OutputExporter.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iostream>

// ==================== Operator Overloading Implementations ====================

// Stream output operator for Move enum
std::ostream& operator<<(std::ostream& os, Move move) {
    switch (move) {
        case Move::Cooperate: 
            os << "C"; 
            break;
        case Move::Defect: 
            os << "D"; 
            break;
        default: 
            os << "?"; 
            break;
    }
    return os;
}

// Stream input operator for Move enum
std::istream& operator>>(std::istream& is, Move& move) {
    char c;
    is >> c;
    switch (c) {
        case 'C': 
        case 'c': 
            move = Move::Cooperate; 
            break;
        case 'D': 
        case 'd': 
            move = Move::Defect; 
            break;
        default: 
            is.setstate(std::ios::failbit); 
            break;
    }
    return is;
}

// Stream output operator for DoubleScoreStats
std::ostream& operator<<(std::ostream& os, const DoubleScoreStats& stats) {
    os << "Mean: " << std::fixed << std::setprecision(2) << stats.mean
       << ", CI: [" << stats.ci_lower << ", " << stats.ci_upper
       << "], StdDev: " << stats.stdev
       << ", N: " << stats.n_samples;
    return os;
}

// Comparison operators for DoubleScoreStats (based on mean score)
bool operator>(const DoubleScoreStats& lhs, const DoubleScoreStats& rhs) {
    return lhs.mean > rhs.mean;
}

bool operator<(const DoubleScoreStats& lhs, const DoubleScoreStats& rhs) {
    return lhs.mean < rhs.mean;
}

bool operator==(const DoubleScoreStats& lhs, const DoubleScoreStats& rhs) {
    return lhs.mean == rhs.mean;
}

bool operator>=(const DoubleScoreStats& lhs, const DoubleScoreStats& rhs) {
    return lhs.mean >= rhs.mean;
}

bool operator<=(const DoubleScoreStats& lhs, const DoubleScoreStats& rhs) {
    return lhs.mean <= rhs.mean;
}

// Stream output operator for LeaderboardEntry
std::ostream& operator<<(std::ostream& os, const LeaderboardEntry& entry) {
    os << "#" << std::setw(3) << std::right << entry.rank << " | "
       << std::setw(25) << std::left << entry.strategy_name << " | "
       << std::fixed << std::setprecision(2) 
       << "Score: " << std::setw(8) << std::right << entry.stats.mean
       << " [" << entry.stats.ci_lower << ", " << entry.stats.ci_upper << "]";
    return os;
}

// ==================== OutputExporter Helper Methods ====================

std::string OutputExporter::formatDouble(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

std::string OutputExporter::escapeJson(const std::string& str) {
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

std::string OutputExporter::escapeCsv(const std::string& str) {
    if (str.find(',') != std::string::npos || str.find('"') != std::string::npos || str.find('\n') != std::string::npos) {
        std::string result = "\"";
        for (char c : str) {
            if (c == '"') result += "\"\"";
            else result += c;
        }
        result += "\"";
        return result;
    }
    return str;
}

void OutputExporter::exportTournamentCSV(
const std::map<std::string, DoubleScoreStats>& results,
const std::string& filename) {
    
std::ofstream file(filename);
if (!file.is_open()) {
    std::cerr << "Error: Cannot open file " << filename << " for writing.\n";
    return;
}
    
// Write header
file << "Strategy,Mean,CI_Lower,CI_Upper,StdDev\n";
    
// Sort by mean score (descending) - using overloaded > operator
std::vector<std::pair<std::string, DoubleScoreStats>> sorted_results(results.begin(), results.end());
std::sort(sorted_results.begin(), sorted_results.end(),
    [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Write data
    for (const auto& [name, stats] : sorted_results) {
        file << escapeCsv(name) << ","
             << formatDouble(stats.mean) << ","
             << formatDouble(stats.ci_lower) << ","
             << formatDouble(stats.ci_upper) << ","
             << formatDouble(stats.stdev) << "\n";
    }
    
    file.close();
    std::cout << "Tournament results exported to: " << filename << "\n";
}

void OutputExporter::exportTournamentJSON(
const std::map<std::string, DoubleScoreStats>& results,
const std::string& filename) {
    
std::ofstream file(filename);
if (!file.is_open()) {
    std::cerr << "Error: Cannot open file " << filename << " for writing.\n";
    return;
}
    
// Sort by mean score (descending) - using overloaded > operator
std::vector<std::pair<std::string, DoubleScoreStats>> sorted_results(results.begin(), results.end());
std::sort(sorted_results.begin(), sorted_results.end(),
    [](const auto& a, const auto& b) { return a.second > b.second; });
    
    file << "{\n";
    file << "  \"tournament_results\": [\n";
    
    for (size_t i = 0; i < sorted_results.size(); ++i) {
        const auto& [name, stats] = sorted_results[i];
        file << "    {\n";
        file << "      \"strategy\": \"" << escapeJson(name) << "\",\n";
        file << "      \"mean\": " << formatDouble(stats.mean, 4) << ",\n";
        file << "      \"ci_lower\": " << formatDouble(stats.ci_lower, 4) << ",\n";
        file << "      \"ci_upper\": " << formatDouble(stats.ci_upper, 4) << ",\n";
        file << "      \"stdev\": " << formatDouble(stats.stdev, 4) << "\n";
        file << "    }";
        if (i < sorted_results.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    std::cout << "Tournament results exported to: " << filename << "\n";
}

void OutputExporter::exportTournamentMarkdown(
const std::map<std::string, DoubleScoreStats>& results,
const std::string& filename) {
    
std::ofstream file(filename);
if (!file.is_open()) {
    std::cerr << "Error: Cannot open file " << filename << " for writing.\n";
    return;
}
    
// Sort by mean score (descending) - using overloaded > operator
std::vector<std::pair<std::string, DoubleScoreStats>> sorted_results(results.begin(), results.end());
std::sort(sorted_results.begin(), sorted_results.end(),
    [](const auto& a, const auto& b) { return a.second > b.second; });
    
    file << "# Tournament Results\n\n";
    file << "| Rank | Strategy | Mean | 95% CI Lower | 95% CI Upper | Std Dev |\n";
    file << "|------|----------|------|--------------|--------------|----------|\n";
    
    int rank = 1;
    for (const auto& [name, stats] : sorted_results) {
        file << "| " << rank++ << " | " << name << " | "
             << formatDouble(stats.mean) << " | "
             << formatDouble(stats.ci_lower) << " | "
             << formatDouble(stats.ci_upper) << " | "
             << formatDouble(stats.stdev) << " |\n";
    }
    
    file.close();
    std::cout << "Tournament results exported to: " << filename << "\n";
}

void OutputExporter::exportNoiseSweepCSV(
const std::map<double, std::map<std::string, DoubleScoreStats>>& results,
const std::string& filename) {
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << " for writing.\n";
        return;
    }
    
    file << "Epsilon,Strategy,Mean,StdDev,CI_Lower,CI_Upper\n";
    
    for (const auto& [epsilon, strategy_results] : results) {
        for (const auto& [strategy, stats] : strategy_results) {
            file << formatDouble(epsilon, 2) << ","
                 << escapeCsv(strategy) << ","
                 << formatDouble(stats.mean) << ","
                 << formatDouble(stats.stdev) << ","
                 << formatDouble(stats.ci_lower) << ","
                 << formatDouble(stats.ci_upper) << "\n";
        }
    }
    
    file.close();
    std::cout << "Noise sweep results exported to: " << filename << "\n";
}

void OutputExporter::exportNoiseSweepJSON(
const std::map<double, std::map<std::string, DoubleScoreStats>>& results,
const std::string& filename) {
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << " for writing.\n";
        return;
    }
    
    file << "{\n";
    file << "  \"noise_sweep_results\": [\n";
    
    size_t eps_idx = 0;
    for (const auto& [epsilon, strategy_results] : results) {
        file << "    {\n";
        file << "      \"epsilon\": " << formatDouble(epsilon, 2) << ",\n";
        file << "      \"strategies\": [\n";
        
        size_t strat_idx = 0;
        for (const auto& [strategy, stats] : strategy_results) {
            file << "        {\n";
            file << "          \"name\": \"" << escapeJson(strategy) << "\",\n";
            file << "          \"mean\": " << formatDouble(stats.mean, 4) << ",\n";
            file << "          \"stdev\": " << formatDouble(stats.stdev, 4) << ",\n";
            file << "          \"ci_lower\": " << formatDouble(stats.ci_lower, 4) << ",\n";
            file << "          \"ci_upper\": " << formatDouble(stats.ci_upper, 4) << "\n";
            file << "        }";
            if (strat_idx < strategy_results.size() - 1) file << ",";
            file << "\n";
            ++strat_idx;
        }
        
        file << "      ]\n";
        file << "    }";
        if (eps_idx < results.size() - 1) file << ",";
        file << "\n";
        ++eps_idx;
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    std::cout << "Noise sweep results exported to: " << filename << "\n";
}

void OutputExporter::exportEvolutionCSV(
    const std::vector<std::map<std::string, double>>& history,
    const std::vector<std::unique_ptr<Strategy>>& strategies,
    const std::string& label,
    const std::string& filename) {
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << " for writing.\n";
        return;
    }
    
    // Write header
    file << "Generation";
    for (const auto& s : strategies) {
        file << "," << escapeCsv(s->getName());
    }
    file << "\n";
    
    // Write data
    for (size_t gen = 0; gen < history.size(); ++gen) {
        file << gen;
        for (const auto& s : strategies) {
            file << "," << formatDouble(history[gen].at(s->getName()), 4);
        }
        file << "\n";
    }
    
    file.close();
    std::cout << "Evolution history (" << label << ") exported to: " << filename << "\n";
}

void OutputExporter::exportEvolutionJSON(
    const std::vector<std::map<std::string, double>>& history,
    const std::vector<std::unique_ptr<Strategy>>& strategies,
    const std::string& label,
    const std::string& filename) {
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << " for writing.\n";
        return;
    }
    
    file << "{\n";
    file << "  \"label\": \"" << escapeJson(label) << "\",\n";
    file << "  \"evolution_history\": [\n";
    
    for (size_t gen = 0; gen < history.size(); ++gen) {
        file << "    {\n";
        file << "      \"generation\": " << gen << ",\n";
        file << "      \"populations\": {\n";
        
        size_t idx = 0;
        for (const auto& s : strategies) {
            file << "        \"" << escapeJson(s->getName()) << "\": "
                 << formatDouble(history[gen].at(s->getName()), 4);
            if (idx < strategies.size() - 1) file << ",";
            file << "\n";
            ++idx;
        }
        
        file << "      }\n";
        file << "    }";
        if (gen < history.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    std::cout << "Evolution history (" << label << ") exported to: " << filename << "\n";
}
