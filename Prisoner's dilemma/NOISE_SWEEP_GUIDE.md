# 噪声扫描功能使用指南 (Noise Sweep Guide)

## 📋 功能说明

噪声扫描功能允许你在不同的噪声水平（epsilon）下系统地测试策略的鲁棒性。这对应于 **Q2: Reciprocity under noise** 的要求。

## 🎯 使用方法

### 方法 1: 使用默认噪声水平

```bash
# 测试 TFT, GRIM, PAVLOV, CTFT 在默认噪声水平下的表现
# 默认: epsilon = 0.0, 0.05, 0.1, 0.15, 0.2
./program --noise-sweep --strategies TitForTat GrimTrigger PAVLOV ContriteTitForTat --rounds 100 --repeats 10
```

### 方法 2: 自定义噪声水平

```bash
# 指定自定义的噪声水平
./program --noise-sweep \
  --strategies TitForTat GrimTrigger PAVLOV ContriteTitForTat \
  --epsilon-values 0.0 0.02 0.05 0.08 0.1 0.15 0.2 \
  --rounds 100 --repeats 20
```

### 方法 3: 包含更多策略

```bash
# 测试所有主要策略
./program --noise-sweep \
  --strategies AllCooperate AllDefect TitForTat GrimTrigger PAVLOV ContriteTitForTat PROBER \
  --rounds 100 --repeats 10
```

## 📊 输出说明

### 1. 控制台表格输出

程序会在控制台显示一个表格，格式如下：

```
=================================================
--- Noise Sweep Analysis Results ---
=================================================

Epsilon (epsilon) | TitForTat | GrimTrigger | PAVLOV | ContriteTitForTat
------------|-----------|-------------|--------|-------------------
0.00        | 285.50    | 280.30      | 275.80 | 283.20
0.05        | 265.40    | 245.60      | 268.90 | 270.10
0.10        | 240.20    | 195.30      | 255.40 | 258.70
0.15        | 215.80    | 152.40      | 235.60 | 242.30
0.20        | 190.50    | 110.20      | 210.30 | 225.40
```

### 2. CSV 文件输出

同时会自动生成 `noise_analysis.csv` 文件，格式为：

```csv
Epsilon,Strategy,Mean,StdDev,CI_Lower,CI_Upper
0.00,TitForTat,285.50,4.20,283.30,287.70
0.00,GrimTrigger,280.30,5.10,277.50,283.10
0.00,PAVLOV,275.80,4.80,273.20,278.40
0.00,ContriteTitForTat,283.20,4.50,280.80,285.60
0.05,TitForTat,265.40,6.50,261.60,269.20
...
```

这个 CSV 文件可以直接导入 Python、R 或 Excel 进行绘图分析。

## 📈 数据可视化 (可选)

### 使用 Python 绘制噪声分析图

创建 `plot_noise_analysis.py`:

```python
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# 设置样式
sns.set_style("whitegrid")
plt.figure(figsize=(12, 7))

# 读取数据
df = pd.read_csv('noise_analysis.csv')

# 为每个策略绘制线条
for strategy in df['Strategy'].unique():
    data = df[df['Strategy'] == strategy]
    plt.plot(data['Epsilon'], data['Mean'], 
             marker='o', linewidth=2, markersize=8, 
             label=strategy)

# 图表设置
plt.xlabel('Noise Level (epsilon)', fontsize=14)
plt.ylabel('Average Payoff', fontsize=14)
plt.title('Strategy Performance vs Noise Level', fontsize=16, fontweight='bold')
plt.legend(fontsize=12, loc='best')
plt.grid(True, alpha=0.3)
plt.tight_layout()

# 保存和显示
plt.savefig('noise_analysis.png', dpi=300, bbox_inches='tight')
plt.show()

print("Plot saved as: noise_analysis.png")
```

运行:
```bash
python plot_noise_analysis.py
```

## 🔍 分析要点

在报告中应该讨论以下内容：

### 1. 哪些策略在噪声下崩溃？

**GRIM (Grim Trigger)** 通常表现最差：
- **原因**: 一旦检测到背叛就永远不恕
- **在噪声下**: 误判的背叛导致永久的报复循环
- **预期行为**: 随着 epsilon 增加，得分急剧下降

### 2. 哪些策略鲁棒（resilient）？

**CTFT (Contrite Tit-For-Tat)** 和 **PAVLOV** 通常表现最好：
- **CTFT 原因**: 有"悔悟"机制，能识别并修复因噪声导致的背叛
- **PAVLOV 原因**: Win-Stay-Lose-Shift 策略能快速恢复合作
- **预期行为**: 得分下降较慢且平稳

**TFT (Tit-For-Tat)** 表现中等：
- **原因**: 会模仿对手，但容易陷入报复循环
- **预期行为**: 得分下降速度居中

### 3. 建议的讨论框架

```
Discussion for Q2:

1. Implementation of Noise Mechanism:
   - Each move has probability epsilon of flipping (C→D or D→C)
   - Tested with epsilon ∈ {0.0, 0.05, 0.1, 0.15, 0.2}

2. Results Summary:
   [插入表格或图表]

3. Key Findings:
   a) GRIM collapsed under noise:
      - At epsilon=0.0: score = 280.30
      - At epsilon=0.2: score = 110.20
      - Drop: 61% (most severe)
      - Reason: Unforgiving nature makes it vulnerable to false triggers

   b) CTFT remained resilient:
      - At epsilon=0.0: score = 283.20
      - At epsilon=0.2: score = 225.40
      - Drop: 20% (least severe)
      - Reason: Contrite mechanism repairs noise-induced defections

   c) PAVLOV showed good robustness:
      - At epsilon=0.0: score = 275.80
      - At epsilon=0.2: score = 210.30
      - Drop: 24%
      - Reason: Win-Stay-Lose-Shift quickly restores cooperation

4. Theoretical Explanation:
   - Strategies with forgiveness/repair mechanisms (CTFT, PAVLOV) 
     maintain cooperation despite implementation errors
   - Unforgiving strategies (GRIM) spiral into mutual defection
   - Simple mirroring (TFT) gets trapped in echo cycles
```

## 🧪 推荐的测试配置

### 配置 1: 快速测试
```bash
./program --noise-sweep \
  --strategies TitForTat GrimTrigger PAVLOV ContriteTitForTat \
  --rounds 50 --repeats 5
```
- 用时: ~1-2 分钟
- 用途: 快速验证功能

### 配置 2: 标准测试（推荐用于报告）
```bash
./program --noise-sweep \
  --strategies TitForTat GrimTrigger PAVLOV ContriteTitForTat \
  --rounds 100 --repeats 10
```
- 用时: ~5-10 分钟
- 用途: 获得可靠的统计数据

### 配置 3: 高精度测试
```bash
./program --noise-sweep \
  --strategies TitForTat GrimTrigger PAVLOV ContriteTitForTat \
  --epsilon-values 0.0 0.01 0.02 0.03 0.04 0.05 0.075 0.1 0.15 0.2 \
  --rounds 200 --repeats 20
```
- 用时: ~20-30 分钟
- 用途: 获得更细粒度的噪声分析曲线

## 📝 报告模板

### 表格格式建议

**Table 1: Average Payoff vs Noise Level**

| epsilon    | TFT    | GRIM   | PAVLOV | CTFT   | Best Strategy |
|------|--------|--------|--------|--------|---------------|
| 0.00 | 285.50 | 280.30 | 275.80 | 283.20 | TFT          |
| 0.05 | 265.40 | 245.60 | 268.90 | 270.10 | CTFT         |
| 0.10 | 240.20 | 195.30 | 255.40 | 258.70 | CTFT         |
| 0.15 | 215.80 | 152.40 | 235.60 | 242.30 | CTFT         |
| 0.20 | 190.50 | 110.20 | 210.30 | 225.40 | CTFT         |

**Table 2: Performance Drop Analysis**

| Strategy | Score at epsilon=0.0 | Score at epsilon=0.2 | Absolute Drop | Percentage Drop |
|----------|----------------|----------------|---------------|-----------------|
| GRIM     | 280.30         | 110.20         | 170.10        | 60.7%          |
| TFT      | 285.50         | 190.50         | 95.00         | 33.3%          |
| PAVLOV   | 275.80         | 210.30         | 65.50         | 23.8%          |
| CTFT     | 283.20         | 225.40         | 57.80         | 20.4%          |

## 🚀 后续步骤

完成噪声扫描后，建议：

1. **Q3: 剥削者测试**
   ```bash
   ./program --exploiters \
     --strategies PROBER AllCooperate TitForTat PAVLOV \
     --epsilon 0.0 --rounds 100
   ```

2. **Q4: 进化模拟**
   ```bash
   ./program --evolve \
     --strategies TitForTat GRIM PAVLOV CTFT \
     --generations 50 --epsilon 0.05
   ```

3. **Q5: SCB 分析**
   ```bash
   ./program --enable-scb --scb-cost 0.5 \
     --strategies AllCooperate TitForTat PAVLOV CTFT PROBER
   ```

## 📞 问题排查

### 问题: 程序运行时间过长
**解决**: 减少 `--rounds` 或 `--repeats` 参数

### 问题: CSV 文件找不到
**解决**: 检查当前工作目录，文件应该在程序运行的目录下

### 问题: 结果不符合预期
**解决**: 
- 检查 `--seed` 是否固定（保证可重复性）
- 增加 `--repeats` 以减少随机性影响
- 确保策略名称拼写正确

## ✅ 验证清单

使用以下清单验证实现是否正确：

- [ ] 程序能成功运行 `--noise-sweep` 模式
- [ ] 控制台输出清晰的表格
- [ ] 生成了 `noise_analysis.csv` 文件
- [ ] CSV 文件格式正确，可以用 Excel 打开
- [ ] GRIM 的得分随噪声增加而急剧下降
- [ ] CTFT/PAVLOV 表现出更好的鲁棒性
- [ ] 可以自定义 `--epsilon-values` 参数

---

**实现完成日期**: 2024
**对应评估问题**: Q2 (10 marks)
**核心功能**: 噪声扫描分析 (Noise Sweep Analysis)
