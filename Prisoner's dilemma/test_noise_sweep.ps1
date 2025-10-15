# 噪声扫描测试脚本
# Noise Sweep Test Script

Write-Host "`n=========================================" -ForegroundColor Cyan
Write-Host "  Q2: 噪声扫描功能测试" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

# 查找可执行文件
$exePath = Get-ChildItem -Path "." -Filter "*.exe" -Recurse | Select-Object -First 1

if ($null -eq $exePath) {
    Write-Host "`n错误: 找不到可执行文件" -ForegroundColor Red
    Write-Host "请先编译项目" -ForegroundColor Yellow
    exit 1
}

Write-Host "`n找到可执行文件: $($exePath.FullName)" -ForegroundColor Green

# 测试 1: 快速测试（5轮，2次重复）
Write-Host "`n=========================================" -ForegroundColor Yellow
Write-Host "  测试 1: 快速验证测试" -ForegroundColor Yellow
Write-Host "=========================================" -ForegroundColor Yellow
Write-Host "配置: rounds=5, repeats=2" -ForegroundColor Gray

& $exePath.FullName `
    --noise-sweep `
    --strategies TitForTat GrimTrigger PAVLOV ContriteTitForTat `
    --rounds 5 `
    --repeats 2

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n✓ 测试 1 通过" -ForegroundColor Green
} else {
    Write-Host "`n✗ 测试 1 失败" -ForegroundColor Red
    exit 1
}

# 测试 2: 标准测试（推荐用于报告）
Write-Host "`n=========================================" -ForegroundColor Yellow
Write-Host "  测试 2: 标准测试（推荐用于报告）" -ForegroundColor Yellow
Write-Host "=========================================" -ForegroundColor Yellow
Write-Host "配置: rounds=100, repeats=10" -ForegroundColor Gray
Write-Host "预计用时: 5-10 分钟" -ForegroundColor Gray

$response = Read-Host "`n是否运行标准测试? (y/n)"

if ($response -eq 'y' -or $response -eq 'Y') {
    & $exePath.FullName `
        --noise-sweep `
        --strategies TitForTat GrimTrigger PAVLOV ContriteTitForTat `
        --rounds 100 `
        --repeats 10
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "`n✓ 测试 2 通过" -ForegroundColor Green
        
        # 检查 CSV 文件
        if (Test-Path "noise_analysis.csv") {
            Write-Host "`n✓ CSV 文件已生成: noise_analysis.csv" -ForegroundColor Green
            
            # 显示文件大小
            $fileSize = (Get-Item "noise_analysis.csv").Length
            Write-Host "  文件大小: $fileSize bytes" -ForegroundColor Gray
            
            # 显示前几行
            Write-Host "`n文件预览:" -ForegroundColor Yellow
            Get-Content "noise_analysis.csv" -TotalCount 10 | Write-Host -ForegroundColor Gray
            
        } else {
            Write-Host "`n⚠ 警告: CSV 文件未生成" -ForegroundColor Yellow
        }
    } else {
        Write-Host "`n✗ 测试 2 失败" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "跳过测试 2" -ForegroundColor Yellow
}

# 测试 3: 自定义噪声水平
Write-Host "`n=========================================" -ForegroundColor Yellow
Write-Host "  测试 3: 自定义噪声水平" -ForegroundColor Yellow
Write-Host "=========================================" -ForegroundColor Yellow

$response = Read-Host "`n是否测试自定义噪声水平? (y/n)"

if ($response -eq 'y' -or $response -eq 'Y') {
    Write-Host "使用自定义噪声值: 0.0, 0.03, 0.07, 0.12, 0.18" -ForegroundColor Gray
    
    & $exePath.FullName `
        --noise-sweep `
        --strategies TitForTat GrimTrigger PAVLOV ContriteTitForTat `
        --epsilon-values 0.0 0.03 0.07 0.12 0.18 `
        --rounds 50 `
        --repeats 5
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "`n✓ 测试 3 通过" -ForegroundColor Green
    } else {
        Write-Host "`n✗ 测试 3 失败" -ForegroundColor Red
    }
}

# Python 可视化
Write-Host "`n=========================================" -ForegroundColor Cyan
Write-Host "  数据可视化" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

if (Test-Path "noise_analysis.csv") {
    $pythonPath = Get-Command python -ErrorAction SilentlyContinue
    
    if ($pythonPath) {
        $response = Read-Host "`n是否运行 Python 绘图脚本? (y/n)"
        
        if ($response -eq 'y' -or $response -eq 'Y') {
            if (Test-Path "plot_noise_analysis.py") {
                Write-Host "`n正在运行 plot_noise_analysis.py..." -ForegroundColor Yellow
                python plot_noise_analysis.py
                
                if ($LASTEXITCODE -eq 0) {
                    Write-Host "`n✓ 图表已生成" -ForegroundColor Green
                    
                    if (Test-Path "noise_analysis.png") {
                        Write-Host "  - noise_analysis.png" -ForegroundColor Gray
                    }
                    if (Test-Path "strategy_comparison.png") {
                        Write-Host "  - strategy_comparison.png" -ForegroundColor Gray
                    }
                }
            } else {
                Write-Host "`n⚠ 警告: 找不到 plot_noise_analysis.py" -ForegroundColor Yellow
            }
        }
    } else {
        Write-Host "`n⚠ Python 未安装，跳过可视化" -ForegroundColor Yellow
        Write-Host "  你可以手动在 Excel 中打开 noise_analysis.csv 进行分析" -ForegroundColor Gray
    }
}

# 总结
Write-Host "`n=========================================" -ForegroundColor Cyan
Write-Host "  测试完成总结" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

Write-Host "`n生成的文件:" -ForegroundColor Yellow

$files = @(
    "noise_analysis.csv",
    "noise_analysis.png",
    "strategy_comparison.png"
)

foreach ($file in $files) {
    if (Test-Path $file) {
        Write-Host "  ✓ $file" -ForegroundColor Green
    } else {
        Write-Host "  ✗ $file (未生成)" -ForegroundColor Gray
    }
}

Write-Host "`n后续步骤:" -ForegroundColor Yellow
Write-Host "  1. 查看 noise_analysis.csv 了解详细数据" -ForegroundColor Gray
Write-Host "  2. 如果有 PNG 文件，查看可视化图表" -ForegroundColor Gray
Write-Host "  3. 在报告中讨论:" -ForegroundColor Gray
Write-Host "     - GRIM 为何在噪声下崩溃" -ForegroundColor Gray
Write-Host "     - CTFT/PAVLOV 为何更鲁棒" -ForegroundColor Gray
Write-Host "     - 噪声对合作维持的影响" -ForegroundColor Gray

Write-Host "`n完成！" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
