# Q3: 对剥削者的稳健性 (Robustness to Exploiters)

## 概述 (Overview)

本实验旨在研究合作型策略如何抵御剥削型策略（PROBER 和 ALLD），以及噪声对这种互动的影响。

## 实现的剥削型策略

### 1. ALLD (Always Defect - 总是背叛)
- **描述**: 无条件背叛策略
- **行为**: 无论对手如何行动，每轮都选择背叛
- **目标**: 获得短期最大收益，特别是对抗无条件合作者
- **弱点**: 无法建立长期合作关系，与大多数策略对战只能获得惩罚收益(P=1.0)

### 2. PROBER (探测者 - 智能剥削策略)
- **描述**: 通过试探来识别可剥削对手的智能策略
- **行为模式**:
  - **第1轮**: 背叛 (D) - 测试对手反应
  - **第2轮**: 合作 (C) - 观察对手是否报复
  - **第3轮**: 合作 (C) - 确认对手行为模式
  - **第4轮起**: 
    - 如果对手在第2、3轮都合作 → 永远背叛（剥削模式）
    - 否则 → 切换到TFT策略（防御/合作模式）
- **智能之处**: 能区分弱势对手和强势对手，选择性剥削
- **目标**: 最大化对弱者的剥削，同时与强者建立合作

## 实验设置

### 参与策略
- **合作型**: ALLC, TFT, GRIM, PAVLOV, CTFT
- **剥削型**: ALLD, PROBER

### 实验参数
- **回合数**: 150轮/场
- **重复次数**: 50次/对战
- **收益矩阵**: T=5.0, R=3.0, P=1.0, S=0.0
- **噪声水平**: 0.0, 0.05, 0.10, 0.15, 0.20

## 运行实验

### 基础实验（无噪声）
```bash
"Prisoner's dilemma.exe" --rounds 150 --repeats 50 --epsilon 0.0 --noise-sweep false --strategies AllCooperate AllDefect TitForTat GrimTrigger PAVLOV CTFT PROBER
```

### PROBER vs ALLC 专项测试
```bash
"Prisoner's dilemma.exe" --rounds 150 --repeats 50 --epsilon 0.0 --noise-sweep false --strategies PROBER AllCooperate
```

### PROBER vs TFT 专项测试
```bash
"Prisoner's dilemma.exe" --rounds 150 --repeats 50 --epsilon 0.0 --noise-sweep false --strategies PROBER TitForTat
```

### 噪声扫描实验
```bash
"Prisoner's dilemma.exe" --rounds 150 --repeats 50 --noise-sweep true --noise-min 0.0 --noise-max 0.2 --noise-step 0.05 --strategies AllCooperate AllDefect TitForTat GrimTrigger PAVLOV CTFT PROBER
```

### 混合种群实验
```bash
"Prisoner's dilemma.exe" --rounds 150 --repeats 50 --epsilon 0.0 --noise-sweep false --strategies AllCooperate TitForTat PAVLOV ALLD PROBER
```

## 预期结果

### 1. PROBER vs ALLC（无噪声）
**预测**: PROBER 完全剥削 ALLC
- PROBER得分: 约 5 + 3 + 3 + 5×147 = 746 分
- ALLC得分: 约 0 + 3 + 3 + 0×147 = 6 分
- PROBER平均每轮: ~4.97
- ALLC平均每轮: ~0.04

**解释**: 
- PROBER在探测期（D-C-C）发现ALLC在第2、3轮都合作
- 判定ALLC为可剥削对象，第4轮起永远背叛
- ALLC无防御机制，持续被剥削

### 2. PROBER vs TFT（无噪声）
**预测**: 部分损失后达成合作
- PROBER得分: 约 5 + 3 + 1 + 3×147 = 450 分
- TFT得分: 约 0 + 3 + 1 + 3×147 = 445 分
- 双方平均每轮: ~3.0

**解释**:
- 第1轮: PROBER(D) vs TFT(C) → PROBER得T=5, TFT得S=0
- 第2轮: PROBER(C) vs TFT(D) → 双方得S=0, T=5（TFT报复）
- 第3轮: PROBER(C) vs TFT(C) → 双方得R=3（恢复合作）
- PROBER检测到TFT有报复能力，切换到TFT模式
- 第4轮起: 双方互相合作，都得R=3

### 3. PROBER vs GRIM（无噪声）
**预测**: 触发永久对抗
- 双方得分: 约 0 + 1×149 = 150 分
- 平均每轮: ~1.0

**解释**:
- 第1轮: PROBER(D) vs GRIM(C) → 触发GRIM永久背叛
- 第2轮起: 双方永远互相背叛，都得P=1

### 4. 噪声环境下的变化
**ε = 0.05 (5%噪声)**:
- GRIM性能严重下降（-50%以上）：偶然错误导致永久背叛循环
- TFT性能中等下降（-20-30%）：短期报复循环
- PROBER探测准确性下降：误判增加
- CTFT/PAVLOV表现最稳健：纠错能力强

**ε = 0.20 (20%噪声)**:
- 所有策略性能显著下降
- CTFT和PAVLOV相对优势更明显
- PROBER的剥削能力大幅降低（探测失效）
- 合作型策略互相对战的优势减少

### 5. 混合种群
**预期排名（无噪声）**:
1. TFT / PAVLOV - 与合作者互惠，有效防御剥削者
2. CTFT - 类似TFT，但稍微宽容
3. PROBER - 成功剥削ALLC，与其他策略部分合作
4. ALLC - 被剥削者严重损害
5. ALLD - 无法建立合作，只得P

**预期排名（高噪声）**:
1. CTFT / PAVLOV - 纠错能力强
2. TFT - 仍然稳健
3. ALLC - 噪声反而减少被剥削程度
4. PROBER - 探测失效
5. ALLD / GRIM - 表现最差

## 分析要点

### 为什么某些策略抗剥削能力更强？

1. **GRIM的优势**:
   - 零容忍政策立即威慑PROBER
   - 首次背叛触发永久报复
   - 但在噪声环境下脆弱

2. **TFT的平衡**:
   - 快速报复（一轮后）
   - 展示反击能力，阻止PROBER剥削
   - 允许恢复合作
   - 噪声环境下仍相对稳健

3. **PAVLOV的智慧**:
   - 基于收益而非行动做决策
   - 能识别不利局面并调整
   - 对噪声有较强适应性

4. **ALLC的脆弱性**:
   - 无防御机制
   - 无法惩罚背叛者
   - 完全暴露在剥削风险中

5. **CTFT的宽容**:
   - 在噪声环境下优于TFT
   - 但可能被智能剥削者利用
   - 权衡了稳健性和合作性

### PROBER的剥削机制

1. **选择性剥削**:
   - 智能区分对手类型
   - 只剥削无防御能力的对手（如ALLC）
   - 与有报复能力的对手合作（如TFT）

2. **探测期设计**:
   - D-C-C模式既能测试又能限制损失
   - 如果对手报复，只损失一轮
   - 如果对手不报复，后续持续剥削

3. **噪声的影响**:
   - 噪声干扰探测准确性
   - 可能误判对手类型
   - 降低整体剥削效果

## 结论

1. **抗剥削性**与**噪声稳健性**往往需要权衡
2. GRIM在无噪声环境下抗剥削性最强，但噪声敏感
3. TFT在两方面取得较好平衡
4. CTFT和PAVLOV在噪声环境下表现最佳
5. PROBER展示了智能剥削的可能性，但依赖准确的信息
6. ALLD虽然简单，但在混合种群中表现不佳

## 实验评分标准

1. **展示PROBER行为** (3分):
   - PROBER vs ALLC：完全剥削
   - PROBER vs TFT：部分损失后合作

2. **展示ALLD在混合种群中的表现** (3分):
   - 对ALLC高分
   - 对其他策略低分（多为P）

3. **解释为什么某些策略抗剥削能力更强** (4分):
   - 报复机制的重要性
   - 宽容性的权衡
   - 噪声环境下的差异

## 扩展实验建议

1. 调整探测期长度，观察PROBER性能变化
2. 测试其他剥削策略（如Suspicious Tit-for-Tat）
3. 研究种群比例对结果的影响
4. 分析进化动态（如果实现进化模块）
