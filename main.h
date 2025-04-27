#include "dblp_reader.h"
#include "apriori.h"
#include "fp_growth.h"
#include "eclat.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <unistd.h>
#include <sstream>
#include <random>

std::string trim(const std::string &str)
{
  size_t first = str.find_first_not_of(" \t\n\r");
  if (first == std::string::npos)
    return "";

  size_t last = str.find_last_not_of(" \t\n\r");

  return str.substr(first, (last - first + 1));
}

// 保存非1项集到文件
void saveItemsetsToFile(const std::vector<std::vector<std::string>> &itemsets,
                        const std::string &filename)
{
  std::ofstream output(filename);
  if (!output.is_open())
  {
    std::cerr << "无法打开文件保存结果: " << filename << std::endl;
    return;
  }

  int count = 0;
  for (const auto &itemset : itemsets)
  {
    if (itemset.size() > 1)
    { // 只保存非1项集
      output << "项集 " << count++ << ": ";
      for (size_t i = 0; i < itemset.size(); i++)
      {
        output << itemset[i];
        if (i < itemset.size() - 1)
        {
          output << ", ";
        }
      }
      output << std::endl;
    }
  }
  output.close();
  std::cout << "非1项集已保存至 " << filename << std::endl;
}

template <typename Func>
auto executeWithTiming(Func func, const std::string &algorithmName)
{
  auto start_time = std::chrono::high_resolution_clock::now();
  auto result = func();
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  std::cout << algorithmName << "结果大小: " << result.size() << std::endl;
  std::cout << algorithmName << "算法运行时间: " << duration.count() << " 毫秒" << std::endl;

  return result;
}

auto getSparseItems = [](const Publication &pub)
{
  std::vector<std::string> items;
  // 只选择作者和年份，产生稀疏的项集
  for (const auto &author : pub.authors)
  {
    items.push_back(trim(author));
  }
  items.push_back(trim(pub.year));
  return items;
};

auto getDenseItems = [](const Publication &pub)
{
  std::vector<std::string> items;
  // 使用所有可用字段，产生密集的项集
  items.push_back(trim(pub.key));
  items.push_back(trim(pub.title));
  items.push_back(trim(pub.venue));
  items.push_back(trim(pub.year));
  for (const auto &author : pub.authors)
  {
    items.push_back(trim(author));
  }
  return items;
};

auto getLongPatternItems = [](const Publication &pub)
{
  std::vector<std::string> items;
  // 将title分词处理，但只取前5个词，减少项的数量
  std::string title = trim(pub.title);
  std::istringstream iss(title);
  std::string word;
  int wordCount = 0;
  while (iss >> word && wordCount < 5)
  {
    items.push_back(word);
    wordCount++;
  }
  // 只添加venue，不添加所有作者
  items.push_back(trim(pub.venue));
  // 只添加第一个作者，如果有的话
  if (!pub.authors.empty())
  {
    items.push_back(trim(pub.authors[0]));
  }
  return items;
};

auto getUltraDenseItems = [](const Publication &pub)
{
  std::vector<std::string> items;

  // // 把 title 分词，加进来
  // std::string title = trim(pub.title);
  // std::istringstream titleStream(title);
  // std::string word;
  // // while (titleStream >> word)
  // // {
  // //   items.push_back(word); // 加个前缀避免混淆
  // // }

  // // 把 venue 字符串按字符分割，加进去
  // std::string venue = trim(pub.venue);
  // // for (char c : venue)
  // // {
  // //   if (!isspace(c))
  // //   {
  // //     items.push_back(std::string("venue_") + c); // 每个字符作为一项
  // //   }
  // // }

  // // 每个作者名字也分词，加进去
  // for (const auto &author : pub.authors)
  // {
  //   // std::istringstream authorStream(trim(author));
  //   // while (authorStream >> word)
  //   // {
  //   //   items.push_back("author_" + word);
  //   // }
  // }

  // // 加年份的每一位数字也当成一项
  // std::string year = trim(pub.year);
  // for (char c : year)
  // {
  //   if (isdigit(c))
  //   {
  //     items.push_back(std::string(1, c));
  //   }
  // }
  // // items.push_back(trim(pub.year));
  // // 添加1-100的随机数作为项
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(1, 20);
  
  // 添加100个1-100的随机数作为项
  for (int i = 0; i < 10; i++) {
    items.push_back("random_" + std::to_string(distrib(gen)));
  }

  return items;
};