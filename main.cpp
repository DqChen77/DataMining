#include "main.h"

void runSingleTest(const std::string &algorithm_name,
                   const std::string &dataset_name,
                   const std::vector<Publication> &publications,
                   int min_sup,
                   const std::function<std::vector<std::string>(const Publication &)> &getItems)
{
  std::cout << "\n=== 测试 " << algorithm_name << " 在 " << dataset_name << " 数据集上 ===" << std::endl;

  if (algorithm_name == "Apriori")
  {
    auto result = executeWithTiming([&]()
                                    { return Apriori<Publication>(publications, min_sup, getItems); }, algorithm_name);
    saveItemsetsToFile(result, "results/" + dataset_name + "_" + algorithm_name + ".txt");
  }
  else if (algorithm_name == "FPGrowth")
  {
    auto result = executeWithTiming([&]()
                                    { return FPGrowth<Publication>(publications, min_sup, getItems); }, algorithm_name);
    saveItemsetsToFile(result, "results/" + dataset_name + "_" + algorithm_name + ".txt");
  }
  else if (algorithm_name == "Eclat")
  {
    auto result = executeWithTiming([&]()
                                    { return Eclat<Publication>(publications, min_sup, getItems); }, algorithm_name);
    saveItemsetsToFile(result, "results/" + dataset_name + "_" + algorithm_name + ".txt");
  }

  sleep(3); // 给内存监控脚本时间记录
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    std::cout << "使用方法: ./dblp_mining <算法名> <数据集类型>" << std::endl;
    std::cout << "算法名: Apriori, FPGrowth, Eclat" << std::endl;
    std::cout << "数据集类型: sparse, dense, long_pattern" << std::endl;
    return 1;
  }

  std::string algorithm = argv[1];
  std::string dataset_type = argv[2];

  DBLPReader reader("data/dblp10000.xml");
  std::vector<Publication> publications = reader.readPublications();
  std::cout << "数据集大小: " << publications.size() << " 条记录" << std::endl;

  int min_sup = 5;

  // 选择数据集处理函数
  std::function<std::vector<std::string>(const Publication &)> getItems;
  if (dataset_type == "sparse")
  {
    getItems = getSparseItems;
  }
  else if (dataset_type == "dense")
  {
    getItems = getDenseItems;
  }
  else if (dataset_type == "long_pattern")
  {
    getItems = getLongPatternItems;
  }
  else if (dataset_type == "ultra_dense")
  {
    getItems = getUltraDenseItems;
  }
  else
  {
    std::cout << "无效的数据集类型" << std::endl;
    return 1;
  }

  runSingleTest(algorithm, dataset_type, publications, min_sup, getItems);

  return 0;
}