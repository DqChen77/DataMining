#ifndef APRIORI_H
#define APRIORI_H

#include <vector>
#include <string>
#include <functional>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iomanip>
#include "publication.h"


template <typename T>
std::vector<std::vector<std::string>> Apriori(const std::vector<T> &data, int min_sup,
                                              std::function<std::vector<std::string>(const T &)> getItems)
{
  std::cout << "开始执行Apriori算法..." << std::endl;

  using FrequentItemsets = std::vector<std::vector<std::vector<std::string>>>;
  FrequentItemsets result;

  std::cout << "步骤1/5: 正在收集所有项并建立TID列表..." << std::endl;
  std::unordered_map<std::string, std::vector<int>> tidLists;
  std::unordered_set<std::string> allItems;

  for (int tid = 0; tid < data.size(); tid++)
  {
    for (const auto &item : getItems(data[tid]))
    {
      // 跳过空项
      if (item.empty())
        continue;

      allItems.insert(item);
      tidLists[item].push_back(tid);
    }
  }
  std::cout << "找到 " << allItems.size() << " 个不同的项" << std::endl;

  std::cout << "步骤2/5: 正在计算每个项的支持度..." << std::endl;
  std::unordered_map<std::string, int> itemSupport;
  int processedItems = 0;

  for (const auto &item : allItems)
  {
    int support = tidLists[item].size();
    if (support >= min_sup)
    {
      itemSupport[item] = support;
    }

    processedItems++;
    if (processedItems % 100 == 0 || processedItems == static_cast<int>(allItems.size()))
    {
      std::cout << "\r处理进度: " << std::fixed << std::setprecision(1)
                << (processedItems * 100.0 / allItems.size()) << "% ("
                << processedItems << "/" << allItems.size() << ")" << std::flush;
    }
  }

  std::cout << std::endl;
  std::cout << "找到 " << itemSupport.size() << " 个项满足最小支持度 " << min_sup << std::endl;

  std::cout << "步骤3/5: 正在创建频繁1项集..." << std::endl;
  std::vector<std::vector<std::string>> L1;
  for (const auto &pair : itemSupport)
  {
    L1.push_back({pair.first});
  }

  if (!L1.empty())
  {
    result.push_back(L1);
    std::cout << "创建了 " << L1.size() << " 个频繁1项集" << std::endl;
  }
  else
  {
    std::cout << "没有找到频繁1项集，算法终止" << std::endl;
    return {};
  }

  auto hasInfrequentSubset = [](const std::vector<std::string> &candidate,
                                const std::vector<std::vector<std::string>> &prevLevel)
  {
    for (int i = 0; i < candidate.size(); i++)
    {
      std::vector<std::string> subset = candidate;
      subset.erase(subset.begin() + i);
      if (std::find(prevLevel.begin(), prevLevel.end(), subset) == prevLevel.end())
      {
        return true;
      }
    }
    return false;
  };

  // 辅助函数：计算两个TID列表的交集大小
  auto intersectionSize = [](const std::vector<int> &list1, const std::vector<int> &list2)
  {
    std::vector<int> intersection;
    std::set_intersection(list1.begin(), list1.end(),
                          list2.begin(), list2.end(),
                          std::back_inserter(intersection));
    return intersection.size();
  };


  std::cout << "步骤4/5: 正在迭代生成频繁项集..." << std::endl;
  int k = 2;
  while (!result.back().empty() && k <= 10)
  {
    std::cout << "  正在生成频繁" << k << "项集..." << std::endl;
    std::vector<std::vector<std::string>> Ck;

    // 生成候选项集并进行剪枝
    size_t totalPairs = result.back().size() * (result.back().size() - 1) / 2;
    size_t processedPairs = 0;
    
    for (size_t i = 0; i < result.back().size(); i++)
    {
      for (size_t j = i + 1; j < result.back().size(); j++)
      {
        bool canMerge = true;
        for (int l = 0; l < k - 2; l++)
        {
          if (result.back()[i][l] != result.back()[j][l])
          {
            canMerge = false;
            break;
          }
        }

        if (canMerge)
        {
          std::vector<std::string> candidate = result.back()[i];
          candidate.push_back(result.back()[j][k - 2]);
          // 剪枝：检查所有k-1子集是否都是频繁的
          if (!hasInfrequentSubset(candidate, result.back()))
          {
            Ck.push_back(candidate);
          }
        }
        
        processedPairs++;
        if (processedPairs % 1000 == 0 || processedPairs == totalPairs) {
          std::cout << "\r  生成候选项集进度: " << std::fixed << std::setprecision(1)
                    << (processedPairs * 100.0 / totalPairs) << "% ("
                    << processedPairs << "/" << totalPairs << ")" << std::flush;
        }
      }
    }
    std::cout << std::endl;

    std::cout << "  生成了 " << Ck.size() << " 个候选" << k << "项集" << std::endl;

    // 计算支持度
    std::vector<std::vector<std::string>> Lk;
    int processedCandidates = 0;
    for (const auto &candidate : Ck)
    {
      // 使用TID列表计算支持度
      std::vector<int> currentTids = tidLists[candidate[0]];
      for (size_t i = 1; i < candidate.size(); i++)
      {
        currentTids = [&]()
        {
          std::vector<int> intersection;
          std::set_intersection(
              currentTids.begin(), currentTids.end(),
              tidLists[candidate[i]].begin(), tidLists[candidate[i]].end(),
              std::back_inserter(intersection));
          return intersection;
        }();

        if (currentTids.size() < min_sup)
        {
          break; // 提前终止
        }
      }

      if (currentTids.size() >= min_sup)
      {
        Lk.push_back(candidate);
      }

      processedCandidates++;
      if (processedCandidates % 10 == 0 || processedCandidates == static_cast<int>(Ck.size()))
      {
        std::cout << "\r  计算支持度进度: " << std::fixed << std::setprecision(1)
                  << (processedCandidates * 100.0 / Ck.size()) << "% ("
                  << processedCandidates << "/" << Ck.size() << ")" << std::flush;
      }
    }
    std::cout << std::endl;

    if (!Lk.empty())
    {
      result.push_back(Lk);
      std::cout << "  找到 " << Lk.size() << " 个频繁" << k << "项集" << std::endl;
    }
    else
    {
      std::cout << "  没有找到频繁" << k << "项集，迭代终止" << std::endl;
      break;
    }

    k++;
  }

  std::cout << "步骤5/5: 正在整理最终结果..." << std::endl;
  std::vector<std::vector<std::string>> finalResult;
  for (const auto &level : result)
  {
    for (const auto &itemset : level)
    {
      finalResult.push_back(itemset);
    }
  }

  std::cout << "Apriori算法完成，共找到 " << finalResult.size() << " 个频繁项集" << std::endl;
  return finalResult;
}

#endif // APRIORI_H
