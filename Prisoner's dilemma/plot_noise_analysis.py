"""
噪声分析可视化脚本
Noise Analysis Visualization Script

使用方法 (Usage):
1. 运行噪声扫描生成 CSV: 
   ./program --noise-sweep --strategies TitForTat GrimTrigger PAVLOV ContriteTitForTat --rounds 100 --repeats 10
   
2. 运行此脚本生成图表:
   python plot_noise_analysis.py
"""

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import sys
import os

def check_file_exists(filename):
    """检查文件是否存在"""
    if not os.path.exists(filename):
        print(f"错误: 找不到文件 '{filename}'")
        print(f"请先运行噪声扫描命令生成数据文件:")
        print(f"  ./program --noise-sweep --strategies TitForTat GrimTrigger PAVLOV ContriteTitForTat")
        return False
    return True

def plot_noise_analysis(csv_file='noise_analysis.csv', output_file='noise_analysis.png'):
    """绘制噪声分析图表"""
    
    # 检查文件
    if not check_file_exists(csv_file):
        return
    
    # 读取数据
    try:
        df = pd.read_csv(csv_file)
        print(f"✓ 成功读取数据文件: {csv_file}")
        print(f"  数据点数: {len(df)}")
        print(f"  策略数: {df['Strategy'].nunique()}")
        print(f"  噪声水平: {sorted(df['Epsilon'].unique())}")
    except Exception as e:
        print(f"错误: 无法读取 CSV 文件: {e}")
        return
    
    # 设置样式
    sns.set_style("whitegrid")
    plt.figure(figsize=(14, 8))
    
    # 定义颜色和标记
    colors = {
        'TitForTat': '#2E86AB',
        'GrimTrigger': '#A23B72',
        'PAVLOV': '#F18F01',
        'ContriteTitForTat': '#C73E1D',
        'AllCooperate': '#6A994E',
        'AllDefect': '#BC4749',
        'PROBER': '#8338EC'
    }
    
    markers = {
        'TitForTat': 'o',
        'GrimTrigger': 's',
        'PAVLOV': '^',
        'ContriteTitForTat': 'D',
        'AllCooperate': 'v',
        'AllDefect': 'p',
        'PROBER': '*'
    }
    
    # 为每个策略绘制线条
    for strategy in df['Strategy'].unique():
        data = df[df['Strategy'] == strategy].sort_values('Epsilon')
        
        color = colors.get(strategy, None)
        marker = markers.get(strategy, 'o')
        
        # 主线条
        plt.plot(data['Epsilon'], data['Mean'], 
                marker=marker, 
                linewidth=2.5, 
                markersize=10, 
                label=strategy,
                color=color,
                alpha=0.8)
        
        # 添加置信区间（如果有）
        if 'CI_Lower' in data.columns and 'CI_Upper' in data.columns:
            plt.fill_between(data['Epsilon'], 
                           data['CI_Lower'], 
                           data['CI_Upper'],
                           alpha=0.15,
                           color=color)
    
    # 图表设置
    plt.xlabel('Noise Level (ε)', fontsize=14, fontweight='bold')
    plt.ylabel('Average Payoff', fontsize=14, fontweight='bold')
    plt.title('Strategy Performance vs Noise Level\n(Iterated Prisoner\'s Dilemma)', 
              fontsize=16, fontweight='bold', pad=20)
    
    plt.legend(fontsize=12, loc='best', frameon=True, shadow=True)
    plt.grid(True, alpha=0.3, linestyle='--')
    plt.tight_layout()
    
    # 保存
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"\n✓ 图表已保存: {output_file}")
    
    # 显示
    plt.show()

def plot_strategy_comparison(csv_file='noise_analysis.csv', output_file='strategy_comparison.png'):
    """绘制策略对比条形图"""
    
    if not check_file_exists(csv_file):
        return
    
    df = pd.read_csv(csv_file)
    
    # 选择几个关键噪声水平
    key_epsilons = [0.0, 0.05, 0.1, 0.2]
    df_filtered = df[df['Epsilon'].isin(key_epsilons)]
    
    # 创建分组条形图
    fig, ax = plt.subplots(figsize=(14, 8))
    
    strategies = df_filtered['Strategy'].unique()
    x = range(len(key_epsilons))
    width = 0.8 / len(strategies)
    
    for i, strategy in enumerate(strategies):
        data = []
        for eps in key_epsilons:
            mean = df_filtered[(df_filtered['Strategy'] == strategy) & 
                              (df_filtered['Epsilon'] == eps)]['Mean'].values
            data.append(mean[0] if len(mean) > 0 else 0)
        
        offset = (i - len(strategies)/2) * width + width/2
        ax.bar([p + offset for p in x], data, width, label=strategy, alpha=0.8)
    
    ax.set_xlabel('Noise Level (ε)', fontsize=14, fontweight='bold')
    ax.set_ylabel('Average Payoff', fontsize=14, fontweight='bold')
    ax.set_title('Strategy Performance Comparison at Different Noise Levels', 
                 fontsize=16, fontweight='bold', pad=20)
    ax.set_xticks(x)
    ax.set_xticklabels([f'{eps:.2f}' for eps in key_epsilons])
    ax.legend(fontsize=11, loc='best')
    ax.grid(True, alpha=0.3, axis='y', linestyle='--')
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"✓ 对比图已保存: {output_file}")
    plt.show()

def print_analysis_table(csv_file='noise_analysis.csv'):
    """打印分析表格"""
    
    if not check_file_exists(csv_file):
        return
    
    df = pd.read_csv(csv_file)
    
    print("\n" + "="*80)
    print("  噪声分析统计表 (Noise Analysis Statistics)")
    print("="*80)
    
    # 按噪声水平分组
    for eps in sorted(df['Epsilon'].unique()):
        print(f"\nε = {eps:.2f}")
        print("-" * 60)
        
        data = df[df['Epsilon'] == eps].sort_values('Mean', ascending=False)
        
        for idx, row in data.iterrows():
            print(f"  {row['Strategy']:20s} | Mean: {row['Mean']:7.2f} | "
                  f"CI: [{row['CI_Lower']:7.2f}, {row['CI_Upper']:7.2f}]")
    
    # 计算性能下降
    print("\n" + "="*80)
    print("  性能下降分析 (Performance Drop Analysis)")
    print("="*80)
    
    strategies = df['Strategy'].unique()
    epsilon_min = df['Epsilon'].min()
    epsilon_max = df['Epsilon'].max()
    
    print(f"\n从 ε={epsilon_min:.2f} 到 ε={epsilon_max:.2f} 的性能变化:")
    print("-" * 80)
    
    drops = []
    for strategy in strategies:
        score_min = df[(df['Strategy'] == strategy) & 
                      (df['Epsilon'] == epsilon_min)]['Mean'].values[0]
        score_max = df[(df['Strategy'] == strategy) & 
                      (df['Epsilon'] == epsilon_max)]['Mean'].values[0]
        
        abs_drop = score_min - score_max
        pct_drop = (abs_drop / score_min) * 100
        
        drops.append({
            'Strategy': strategy,
            'Initial': score_min,
            'Final': score_max,
            'Absolute Drop': abs_drop,
            'Percentage Drop': pct_drop
        })
    
    # 按百分比降幅排序
    drops.sort(key=lambda x: x['Percentage Drop'], reverse=True)
    
    for d in drops:
        print(f"  {d['Strategy']:20s} | "
              f"Initial: {d['Initial']:7.2f} → Final: {d['Final']:7.2f} | "
              f"Drop: {d['Absolute Drop']:7.2f} ({d['Percentage Drop']:5.1f}%)")
    
    print("\n" + "="*80)

def main():
    """主函数"""
    print("\n" + "="*80)
    print("  Prisoner's Dilemma - Noise Analysis Visualization")
    print("="*80 + "\n")
    
    csv_file = 'noise_analysis.csv'
    
    # 检查是否有命令行参数
    if len(sys.argv) > 1:
        csv_file = sys.argv[1]
    
    # 1. 打印统计表格
    print_analysis_table(csv_file)
    
    # 2. 绘制主图表
    print("\n正在生成图表...")
    plot_noise_analysis(csv_file, 'noise_analysis.png')
    
    # 3. 绘制对比图
    print("\n正在生成对比图...")
    plot_strategy_comparison(csv_file, 'strategy_comparison.png')
    
    print("\n" + "="*80)
    print("  分析完成！")
    print("="*80)
    print("\n生成的文件:")
    print("  1. noise_analysis.png      - 噪声水平 vs 平均收益曲线图")
    print("  2. strategy_comparison.png - 策略对比条形图")
    print("\n")

if __name__ == '__main__':
    main()
