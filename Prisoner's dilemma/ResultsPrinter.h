#ifndef RESULTSPRINTER_H
#define RESULTSPRINTER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Config.h"
#include "Strategy.h"
#include "Simulator.h"

/**
 * @class ResultsPrinter
 * @brief 负责所有输出和打印功能的类
 * 
 * 该类将 SimulatorRunner 中的所有打印函数集中管理，
 * 使代码更加模块化和易于维护。
 */
class ResultsPrinter {
public:
    explicit ResultsPrinter(const Config& config);

    // ==================== 配置和矩阵打印 ====================
    
    /// 打印模拟器配置信息
    void printConfiguration(const std::vector<std::unique_ptr<Strategy>>& strategies) const;
    
    /// 打印收益矩阵
    void printPayoffMatrix() const;

    // ==================== 锦标赛结果打印 ====================
    
    /// 打印锦标赛结果表格
    void printTournamentResults(const std::map<std::string, ScoreStats>& results) const;
    
    /// 打印策略分析
    void printAnalysis(const std::string& analysis_text) const;

    // ==================== 剥削者模式打印 ====================
    
    /// 打印剥削者对战表格
    void printExploiterMatchTable(
        const std::string& exploiter_name,
        const std::map<std::string, std::pair<double, double>>& matchAverages) const;
    
    /// 打印剥削者测试结果（完整版）
    void printExploiterResults(
        const std::string& exploiter_name,
        const std::map<std::string, std::pair<ScoreStats, ScoreStats>>& results) const;

    // ==================== 进化模拟打印 ====================
    
    /// 打印进化模拟标题
    void printEvolutionHeader() const;
    
    /// 打印单代种群信息
    void printGeneration(
        int gen, 
        const std::map<std::string, double>& populations,
        const std::vector<std::unique_ptr<Strategy>>& strategies) const;
    
    /// 打印完整的进化历史表格（在所有代完成后）
    void printEvolutionHistory(
        const std::vector<std::map<std::string, double>>& history,
        const std::vector<std::unique_ptr<Strategy>>& strategies,
        const std::string& label) const;
    
    /// 打印详细的单代信息（包含适应度）
    void printGenerationDetailed(
        int gen,
        const std::map<std::string, double>& populations,
        const std::map<std::string, double>& fitness,
        double avg_fitness,
        const std::vector<std::unique_ptr<Strategy>>& strategies) const;

    // ==================== SCB (Strategic Complexity Budget) 打印 ====================
    
    /// 打印策略复杂度信息表
    void printComplexityTable(const std::vector<std::unique_ptr<Strategy>>& strategies) const;
    
    /// 打印 SCB 对比结果（有/无 SCB 的锦标赛结果对比）
    void printSCBComparison(
        const std::map<std::string, ScoreStats>& results_without_scb,
        const std::map<std::string, ScoreStats>& results_with_scb) const;
    
    /// 打印 SCB 影响分析
    void printSCBAnalysis(
        const std::map<std::string, ScoreStats>& results_without_scb,
        const std::map<std::string, ScoreStats>& results_with_scb,
        const std::vector<std::unique_ptr<Strategy>>& strategies) const;

    // ==================== 辅助函数 ====================
    
    /// 格式化 double 值为字符串（2位小数）
    static std::string formatDouble(double value);
    
    /// 格式化 double 值为字符串（指定精度）
    static std::string formatDouble(double value, int precision);

private:
    const Config& config_;
};

#endif // RESULTSPRINTER_H
