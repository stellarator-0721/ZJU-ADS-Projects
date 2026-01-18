# Dijkstra-Heap-最短路径算法性能分析

## 项目简介

本项目基于经典的 Dijkstra 算法，针对单源最短路径问题（SSP）进行实现与性能分析。通过实现两种不同的最小优先队列数据结构（Fibonacci 堆和 Pairing 堆），在真实路网数据集上对比它们作为 Dijkstra 算法优先队列时的实际性能差异。

项目使用 DIMACS 第9届挑战赛的 USA 路网数据集作为基准测试平台，通过大量随机查询对算法性能进行量化评估。实验结果显示，尽管 Fibonacci 堆在理论上具有更优的摊还时间复杂度，但在实际稀疏路网中，Pairing 堆因更小的常数因子和更好的缓存局部性而表现更佳。

## 主要功能

### 1. 核心算法实现
- **Dijkstra 算法**：实现了经典的单源最短路径算法
- **懒插入策略**：仅源节点执行插入操作，其他节点首次通过 `Decrease-Key` 引入
- **图表示**：使用邻接表存储稀疏路网，支持双向搜索

### 2. 堆数据结构实现
- **Fibonacci 堆**：
  - 摊还时间复杂度：`Insert` O(1), `Extract-Min` O(log n), `Decrease-Key` O(1)
  - 复杂的数据结构，包含级联剪枝操作
  - 实现节点映射表以支持 O(1) 节点查找

- **Pairing 堆**：
  - 摊还时间复杂度：`Insert` O(1), `Extract-Min` O(log n), `Decrease-Key` O(log n)
  - 结构简单，常数因子小，缓存友好
  - 使用两遍合并策略（左到右配对，右到左归并）

### 3. 测试框架
- **多种测试模式**：
  - 文件模式：从指定文件读取查询对
  - 随机模式：生成随机查询对进行测试
  - 快速模式：使用预设的10个有保证路径的查询对（推荐）
- **自动化性能测试**：
  - 支持1000+查询对的批量测试
  - 六次重复实验取平均值
  - 结果一致性验证

### 4. 性能评估
- **多数据集测试**：从 NY（1.4万节点）到 USA（135万节点）共11个路网
- **详细指标收集**：
  - 总执行时间
  - 平均查询时间
  - 性能对比分析
- **可视化输出**：生成性能对比图表和详细报告

### 5. 实用工具
- **查询生成器**：自动生成各类测试查询
- **Makefile 自动化**：简化编译和测试流程
- **超时保护**：每查询10秒超时，防止程序挂起
- **错误处理**：自动跳过无效节点和异常情况

## 先决条件

### 1. 硬件要求
- **CPU**：推荐现代多核处理器（测试使用 Intel Core i9-13900）
- **内存**：至少 16GB RAM（USA数据集需要约 8GB 内存）
- **存储**：至少 2GB 可用空间（用于存储数据集和中间文件）

### 2. 软件要求
- **操作系统**：
  - Windows (MinGW-w64 环境)
  - Linux/macOS (GCC 编译器)
- **编译器**：
  - MinGW-w64 GCC (Windows)
  - GCC 或 Clang (Linux/macOS)
  - 支持 C99 标准
- **构建工具**：
  - Make (mingw32-make for Windows)
- **数据集**：
  - DIMACS 9th Challenge USA 路网数据集
  - 下载地址：http://www.dis.uniroma1.it/challenge9/download.shtml

### 3. 数据集准备
将下载的 DIMACS `.gr` 文件放置在项目 `data/` 目录下：
```
data/
├── USA-road-d.NY.gr
├── USA-road-d.BAY.gr
├── USA-road-d.COL.gr
├── USA-road-d.FLA.gr
├── USA-road-d.NW.gr
├── USA-road-d.NE.gr
├── USA-road-d.CAL.gr
├── USA-road-d.LKS.gr
├── USA-road-d.E.gr
├── USA-road-d.W.gr
├── USA-road-d.CTR.gr
└── USA-road-d.USA.gr
```

### 4. 依赖库
- 标准 C 库（stdlib.h, stdio.h, time.h, math.h）
- 无外部第三方库依赖

### 5. 环境配置
**Windows 用户**：
```bash
# 安装 MinGW-w64
# 确保 mingw32-make 在系统 PATH 中
# 验证环境
gcc --version
mingw32-make --version
```

**Linux/macOS 用户**：
```bash
# 安装 GCC 和 Make
sudo apt-get install gcc make  # Ubuntu/Debian
brew install gcc make         # macOS

# 验证环境
gcc --version
make --version
```

### 6. 项目结构检查
克隆项目后，确保目录结构如下：
```
Dijkstra_Heap_Benchmark/
├── include/          # 头文件
├── src/             # 源代码
├── data/            # 数据集（需手动添加）
├── Queries/         # 查询文件（自动生成）
├── bin/             # 可执行文件
├── build/           # 编译中间文件
├── Makefile         # 构建脚本
└── README.md        # 项目说明
```

## 快速开始

1. **编译项目**：
```bash
mingw32-make
```

2. **生成测试查询**：
```bash
mingw32-make generate_queries
```

3. **运行快速测试**（推荐）：
```bash
mingw32-make test_quick
```

4. **查看详细使用说明**：
```bash
mingw32-make help
```

## 性能特点

- **稀疏图优化**：针对路网图特征优化，`E ≈ O(V)`
- **实际效率优先**：虽然 Fibonacci 堆理论上 `Decrease-Key` 为 O(1)，但 Pairing 堆在实际测试中更快
- **工程化实现**：包含内存管理、超时保护、错误处理等工程实践
- **可重复性**：使用固定随机种子确保实验结果可重复

## 适用场景

- 大规模路网最短路径计算
- 优先队列数据结构性能研究
- 理论复杂度与实际性能对比分析
- 算法课程教学与实验

## 注意事项

1. 首次运行需要下载并放置数据集到 `data/` 目录
2. USA 数据集较大（135万节点），需要足够内存
3. 推荐使用 `small_test_queries_10.txt` 进行快速测试
4. 程序包含 10 秒超时保护，防止单个查询耗时过长

通过本项目，你可以深入了解不同堆数据结构在真实场景下的性能表现，验证理论复杂度与实际运行时间之间的关系，并为实际应用中选择合适的优先队列实现提供参考依据。
