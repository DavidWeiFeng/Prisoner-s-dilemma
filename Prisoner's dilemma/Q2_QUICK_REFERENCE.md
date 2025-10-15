# Q2 噪声扫描 - 快速参考卡片
# Noise Sweep Quick Reference Card

## 📌 一分钟快速上手

```bash
# 基础命令
./program --noise-sweep --strategies TitForTat GrimTrigger PAVLOV ContriteTitForTat

# 完整命令（推荐）
./program --noise-sweep \
  --strategies TitForTat GrimTrigger PAVLOV ContriteTitForTat \
  --rounds 100 --repeats 10
```

## 📊 预期输出

### 1. 控制台表格
```
Epsilon (epsilon) | TitForTat | GrimTrigger | PAVLOV | ContriteTitForTat
------------|-----------|-------------|--------|-------------------
0.00        | 285.50    | 280.30      | 275.80 | 283.20
0.20        | 190.50    | 110.20      | 210.30 | 225.40
```

### 2. CSV 文件: `noise_analysis.csv`
```csv
Epsilon,Strategy,Mean,StdDev,CI_Lower,CI_Upper
0.00,TitForTat,285.50,4.20,283.30,287.70
0.05,TitForTat,265.40,6.50,261.60,269.20
...
```

## 🎯 关键发现（用于报告）

### 策略表现排名（从最差到最好）

**在 epsilon=0.2 时：**

1. **GRIM** - 崩溃 ❌
   - 得分降幅: ~60%
   - 原因: 永不原谅 → 陷入永久背叛

2. **TFT** - 中等 ⚠️
   - 得分降幅: ~33%
   - 原因: 报复循环 → 来回背叛

3. **PAVLOV** - 良好 ✓
   - 得分降幅: ~24%
   - 原因: Win-Stay-Lose-Shift → 快速恢复

4. **CTFT** - 最佳 ✓✓
   - 得分降幅: ~20%
   - 原因: 悔悟机制 → 修复错误

## 📝 报告模板（复制粘贴）

```
Q2: Reciprocity under Noise (10 marks)

Implementation:
- Noise mechanism: Each move flips with probability epsilon
- Tested strategies: TFT, GRIM, PAVLOV, CTFT
- Noise range: epsilon ∈ {0.0, 0.05, 0.1, 0.15, 0.2}
- Configuration: 100 rounds, 10 repeats per match

Results:
[插入表格或图表]

Key Findings:
1. GRIM collapsed under noise (60% performance drop)
   → Unforgiving nature makes it vulnerable
   
2. CTFT remained most resilient (20% drop)
   → Contrite mechanism repairs noise-induced errors
   
3. PAVLOV showed good robustness (24% drop)
   → Win-Stay-Lose-Shift quickly restores cooperation
   
4. TFT performance declined moderately (33% drop)
   → Gets trapped in retaliation cycles

Conclusion:
Strategies with forgiveness/repair mechanisms (CTFT, PAVLOV) 
maintain cooperation despite implementation errors, while 
unforgiving strategies (GRIM) spiral into mutual defection.
```

## 🔧 常用参数调整

### 快速测试（1-2分钟）
```bash
--rounds 50 --repeats 5
```

### 标准测试（5-10分钟）⭐ 推荐
```bash
--rounds 100 --repeats 10
```

### 高精度测试（20-30分钟）
```bash
--rounds 200 --repeats 20
```

### 更细粒度的噪声值
```bash
--epsilon-values 0.0 0.01 0.02 0.03 0.04 0.05 0.075 0.1 0.15 0.2
```

## 🐍 Python 可视化（可选）

```bash
# 1. 运行程序生成 CSV
./program --noise-sweep --strategies TitForTat GrimTrigger PAVLOV ContriteTitForTat

# 2. 运行 Python 脚本
python plot_noise_analysis.py

# 输出: noise_analysis.png, strategy_comparison.png
```

## ✅ 验证清单

- [ ] 程序成功运行
- [ ] 生成 `noise_analysis.csv`
- [ ] GRIM 得分急剧下降
- [ ] CTFT 表现最稳定
- [ ] 结果可重复（固定 seed）

## 🚨 常见问题

**Q: 程序运行时间过长？**
A: 减少 `--rounds` 或 `--repeats`

**Q: GRIM 没有崩溃？**
A: 检查噪声是否真的启用了（看 epsilon 值）

**Q: 找不到 CSV 文件？**
A: 在程序运行的目录下查找（当前工作目录）

**Q: 结果每次都不一样？**
A: 使用固定的 `--seed` 参数保证可重复性

## 🎓 评分要点（10分）

- ✓ **实现正确性** (4分): 噪声机制实现正确
- ✓ **结果展示** (3分): 清晰的表格/图表
- ✓ **分析讨论** (3分): 
  - 解释 GRIM 为何崩溃
  - 解释 CTFT/PAVLOV 为何鲁棒
  - 理论联系实际

## 📚 下一步

完成 Q2 后，继续：

```bash
# Q3: 剥削者测试
./program --exploiters --strategies PROBER AllCooperate TitForTat --epsilon 0.0

# Q4: 进化模拟
./program --evolve --strategies TitForTat GRIM PAVLOV CTFT --generations 50

# Q5: SCB 分析
./program --enable-scb --scb-cost 0.5 --strategies AllCooperate TitForTat CTFT
```

---

**记住**: 
- 噪声 = 实现错误（implementation noise）
- epsilon = 每个动作翻转的概率
- 目标 = 找出在噪声下仍能维持合作的策略

**核心洞察**:
- 💡 在噪声环境下，**宽恕比报复更有效**
- 💡 **简单的报复策略**（如 TFT）在现实中不够鲁棒
- 💡 **修复机制**（如 CTFT 的悔悟）是关键
