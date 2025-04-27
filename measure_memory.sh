#!/bin/bash

# 定义三种数据类型
data_types=("sparse" "dense" "long_pattern" "ultra_dense")
algorithms=("Apriori" "FPGrowth" "Eclat")

# 创建结果文件
result_file="memory_results.txt"
echo "算法 | 数据类型 | 最大内存使用量" > $result_file
echo "--- | --- | ---" >> $result_file

for data_type in "${data_types[@]}"; do
  for algorithm in "${algorithms[@]}"; do
    # 如果是Apriori算法且数据类型是ultra_dense，则跳过
    if [ "$algorithm" == "Apriori" ] && [ "$data_type" == "ultra_dense" ]; then
      echo "跳过 $algorithm 算法的 $data_type 数据类型测试..."
      continue
    fi
    
    echo "运行 $algorithm 算法，使用 $data_type 数据类型..."
    
    # 清空日志文件
    > log.txt
    
    # 运行程序，传递算法和数据类型参数
    ./dblp_mining $algorithm $data_type &
    
    # 获取进程ID
    pid=$!
    
    # 监控内存使用
    while kill -0 $pid 2>/dev/null; do
      ps -o rss= -p "$pid" | awk '{print $1/1024" MB"}' >> log.txt
      sleep 0.5
    done
    
    # 读取最大内存值并记录
    max_memory=$(grep -o '[0-9]*\.[0-9]* MB' log.txt | sort -nr | head -1)
    # 修复变量替换问题
    data_type_cap=$(echo "$data_type" | sed 's/\(.\)/\u\1/')
    echo "$algorithm | get${data_type_cap}Items | $max_memory" >> $result_file
    echo "最大内存使用量: $max_memory"
    
    # 等待一下再进行下一次测试
    sleep 2
  done
done

echo "所有测试完成，结果保存在 $result_file 文件中"