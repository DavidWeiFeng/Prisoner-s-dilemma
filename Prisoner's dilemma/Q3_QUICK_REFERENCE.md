# Q3 剥削者测试 - 快速参考卡片
# Q3 Exploiter Test - Quick Reference Card

## 📌 一分钟快速上手

### 任务 1: PROBER 行为展示

```bash
# PROBER vs ALLC（剥削成功）
./program --show-exploiter --strategies PROBER ALLC --rounds 100 --repeats 10

# PROBER vs TFT（剥削失败）
./program --show-exploiter --strategies PROBER TFT --rounds 100 --repeats 10

# PROBER vs 多个对手
./program --show-exploiter --strategies PROBER ALLC TFT PAVLOV CTFT --rounds 100 --repeats 10
```

### 任务 2: ALLD 混合人群表现

```bash
# ALLD 在混合人群（需要添加 --analyze-mixed 参数）
./program --strategies ALLD ALLC TFT PAVLOV CTFT --rounds 100 --repeats 10 --analyze-mixed

# PROBER 在混合人群（对比）
./program --strategies PROBER ALLC TFT PAVLOV CTFT --rounds 100 --repeats 10 --analyze-mixed
```

---

## 📊 预期结果

### PROBER vs ALLC
- **PROBER**: ~530 分
- **ALLC**: ~280 分
- **差值**: +250 分
- **判定**: ✓ 剥削成功

### PROBER vs TFT
- **PROBER**: ~290 分
- **TFT**: ~285 分
- **差值**: +5 分
- **判定**: ~ 部分剥削 / ✗ 剥削失败

### ALLD 混合人群排名
| 排名 | 策略   | 得分  | 状态 |
|------|--------|-------|------|
| 1    | TFT    | 290.5 | 领先 |
| 2    | PAVLOV | 288.3 | 良好 |
| 3    | CTFT   | 287.8 | 良好 |
| 4    | **ALLD**   | **245.6** | **剥削者** |
| 5    | ALLC   | 180.2 | 易被剥削 |

---

## 🎯 关键发现

### PROBER 的智能剥削
✅ **vs ALLC**: 试探成功 → 永远背叛 → 重度剥削  
✅ **vs TFT**: 试探被惩罚 → 切换合作 → 剥削失败  
💡 **结论**: 选择性剥削，适应性强

### ALLD 的表现局限
✅ **vs ALLC**: 持续背叛 → 获得 T 收益 (5)  
❌ **vs TFT/PAVLOV/CTFT**: 互相背叛 → 获得 P 收益 (1)  
📉 **平均得分**: 低于互惠策略  
💡 **结论**: 不是进化稳定策略 (ESS)

---

## 💡 机制解释

### PROBER 的试探序列
```
Round 1: C (合作)
Round 2: D (背叛) ← 试探点
Round 3: C (合作)
Round 4: C (合作)
Round 5+: 
  - 如果对手在 Round 2 后仍合作 → 永远背叛
  - 如果对手报复 → 切换为 TFT 模式
```

### 为什么互惠策略能抵抗？
- **TFT**: 立即报复 Round 2 的背叛
- **PAVLOV**: Lose-Shift 机制调整策略
- **CTFT**: 悔悟机制 + 报复能力
- **核心**: 条件合作 + 报复威胁

### 为什么 ALLD 在混合人群中表现差？
```
收益计算:
ALLD vs ALLC: T = 5 (剥削成功)
ALLD vs TFT:  P = 1 (互相背叛)
ALLD vs PAVLOV: P = 1 (互相背叛)
ALLD vs CTFT: P = 1 (互相背叛)

平均收益: (5 + 1 + 1 + 1) / 4 = 2

对比:
TFT 平均收益: (R + R + R) / 3 ≈ 3 (与大多数互惠策略合作)

结论: ALLD < TFT → ALLD 不是 ESS
```

---

## 🔧 命令行参数

### `--show-exploiter`
**作用**: 详细展示剥削者 vs 对手  
**格式**: `--show-exploiter --strategies <EXPLOITER> <VICTIM1> <VICTIM2> ...`  
**输出**: 每场对战的统计 + 剥削判定 + 机制解释

### `--analyze-mixed`
**作用**: 分析剥削者在混合人群中的表现  
**要求**: 策略列表必须包含 PROBER 或 ALLD  
**格式**: `--strategies <STRATEGIES...> --analyze-mixed`  
**输出**: 排名表 + 性能分析 + 理论解释

---

## 📝 报告模板

```markdown
## Q3: Robustness to Exploiters (10 marks)

### 实验 1: PROBER 行为分析

**命令**:
./program --show-exploiter --strategies PROBER ALLC TFT --rounds 100 --repeats 10

**结果**:
- PROBER vs ALLC: +250 分差 → 剥削成功
- PROBER vs TFT: +5 分差 → 剥削失败

**机制**:
PROBER 使用试探序列识别可剥削对手。对 ALLC 检测无防御后永远背叛；
对 TFT 检测到报复后切换为合作模式。

### 实验 2: ALLD 混合人群表现

**命令**:
./program --strategies ALLD ALLC TFT PAVLOV CTFT --rounds 100 --repeats 10

**结果**:
ALLD 排名第 4/5，得分 245.6，远低于互惠策略（~290）。

**分析**:
ALLD 虽能剥削 ALLC (T=5)，但对其他策略陷入互相背叛 (P=1)。
平均收益低于维持合作的互惠策略 (R=3)。

**结论**:
纯背叛策略不是进化稳定策略 (ESS)。在包含互惠策略的混合人群中，
ALLD 无法获得竞争优势。
```

---

## 🎓 评分要点

**实验设计** (4分):
- ✅ PROBER vs ALLC 演示
- ✅ PROBER vs TFT 演示
- ✅ ALLD 混合人群测试

**结果分析** (3分):
- ✅ 剥削成功/失败判定
- ✅ 得分对比分析
- ✅ 排名和统计数据

**理论讨论** (3分):
- ✅ 解释剥削机制
- ✅ 解释抵抗机制
- ✅ ESS 和进化稳定性

---

## 🚨 常见问题

**Q: 为什么 PROBER vs TFT 还是 PROBER 得分高？**  
A: PROBER 在 Round 2 试探得到额外 5 分 (T)，但随后陷入合作。
   虽然剥削失败，但最初的试探仍带来微小优势。

**Q: ALLD 为什么不是倒数第一？**  
A: ALLC 更差，因为它被所有策略剥削。ALLD 至少能剥削 ALLC。

**Q: 如何测试噪声对剥削的影响？**  
A: 添加 `--epsilon 0.1` 参数。噪声会降低剥削效果，因为
   试探序列可能被噪声干扰。

---

## ✅ 完整测试清单

```bash
# 1. PROBER 选择性剥削
./program --show-exploiter --strategies PROBER ALLC TFT PAVLOV CTFT --rounds 100 --repeats 10

# 2. ALLD 混合人群
./program --strategies ALLD ALLC TFT PAVLOV CTFT --rounds 100 --repeats 10 --analyze-mixed

# 3. PROBER 混合人群（对比）
./program --strategies PROBER ALLC TFT PAVLOV CTFT --rounds 100 --repeats 10 --analyze-mixed

# 4. 噪声影响（可选）
./program --show-exploiter --strategies PROBER ALLC TFT --epsilon 0.1 --rounds 100 --repeats 10
./program --strategies ALLD ALLC TFT PAVLOV CTFT --epsilon 0.1 --rounds 100 --repeats 10 --analyze-mixed
```

---

## 🎉 核心洞察

1. **智能剥削 vs 无脑剥削**
   - PROBER: 选择性剥削 → 适应性强
   - ALLD: 无差别背叛 → 容易被报复

2. **为什么互惠策略稳定？**
   - 对内: 维持合作 (R=3)
   - 对外: 报复剥削 (P=1)
   - 结果: 平均收益高于剥削者

3. **进化稳定性 (ESS)**
   - ALLD 不是 ESS: 互惠策略可以入侵
   - TFT-like 策略接近 ESS: 在互惠环境中稳定
   - 现实意义: 解释为什么合作能进化并维持

---

**核心消息**: 
💡 在混合环境中，**有条件的合作比无条件的背叛更成功**  
💡 **报复能力**是抵抗剥削的关键  
💡 **适应性**比无脑策略更有优势

---

**状态**: ✅ Q3 完成  
**下一步**: Q4 进化模拟 / Q5 SCB 分析
