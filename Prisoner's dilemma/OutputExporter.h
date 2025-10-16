#ifndef OUTPUTEXPORTER_H
#define OUTPUTEXPORTER_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <iostream>
#include "Strategy.h"
#include "Simulator.h"

// Forward declarations for operator overloading
std::ostream& operator<<(std::ostream& os, Move move);
std::istream& operator>>(std::istream& is, Move& move);

// Stream output operator for DoubleScoreStats
std::ostream& operator<<(std::ostream& os, const DoubleScoreStats& stats);

// Comparison operators for DoubleScoreStats (for leaderboard ranking)
bool operator>(const DoubleScoreStats& lhs, const DoubleScoreStats& rhs);
bool operator<(const DoubleScoreStats& lhs, const DoubleScoreStats& rhs);
bool operator==(const DoubleScoreStats& lhs, const DoubleScoreStats& rhs);
bool operator>=(const DoubleScoreStats& lhs, const DoubleScoreStats& rhs);
bool operator<=(const DoubleScoreStats& lhs, const DoubleScoreStats& rhs);

/**
 * @struct LeaderboardEntry
 * @brief Represents a single entry in the tournament leaderboard
 */
struct LeaderboardEntry {
    std::string strategy_name;
    DoubleScoreStats stats;
    int rank;
    
    LeaderboardEntry() : strategy_name(""), stats(), rank(0) {}
    
    LeaderboardEntry(const std::string& name, const DoubleScoreStats& s, int r)
        : strategy_name(name), stats(s), rank(r) {}
    
    // Comparison operator for sorting (higher score = lower rank number = better)
    bool operator<(const LeaderboardEntry& other) const {
        return stats > other.stats; // Higher mean score comes first
    }
    
    // Stream output for console display
    friend std::ostream& operator<<(std::ostream& os, const LeaderboardEntry& entry);
};

/**
 * @class OutputExporter
 * @brief Handles exporting simulation results to different formats (CSV, JSON, Markdown)
 */
class OutputExporter {
public:
// Export tournament results to CSV
static void exportTournamentCSV(
    const std::map<std::string, DoubleScoreStats>& results,
    const std::string& filename);
    
// Export tournament results to JSON
static void exportTournamentJSON(
    const std::map<std::string, DoubleScoreStats>& results,
    const std::string& filename);
    
// Export tournament results to Markdown
static void exportTournamentMarkdown(
    const std::map<std::string, DoubleScoreStats>& results,
    const std::string& filename);
    
// Export noise sweep results to CSV
static void exportNoiseSweepCSV(
    const std::map<double, std::map<std::string, DoubleScoreStats>>& results,
    const std::string& filename);
    
// Export noise sweep results to JSON
static void exportNoiseSweepJSON(
    const std::map<double, std::map<std::string, DoubleScoreStats>>& results,
    const std::string& filename);
    
    // Export evolution history to CSV
    static void exportEvolutionCSV(
        const std::vector<std::map<std::string, double>>& history,
        const std::vector<std::unique_ptr<Strategy>>& strategies,
        const std::string& label,
        const std::string& filename);
    
    // Export evolution history to JSON
    static void exportEvolutionJSON(
        const std::vector<std::map<std::string, double>>& history,
        const std::vector<std::unique_ptr<Strategy>>& strategies,
        const std::string& label,
        const std::string& filename);
    
private:
    // Helper to format double values
    static std::string formatDouble(double value, int precision = 2);
    
    // Helper to escape JSON strings
    static std::string escapeJson(const std::string& str);
    
    // Helper to escape CSV fields
    static std::string escapeCsv(const std::string& str);
};

#endif // OUTPUTEXPORTER_H
