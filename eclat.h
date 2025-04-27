#ifndef ECLAT_H
#define ECLAT_H

#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iomanip>
#include <set>
#include "publication.h"

template <typename T>
std::vector<std::vector<std::string>> Eclat(const std::vector<T> &data, int min_sup,
                                           std::function<std::vector<std::string>(const T &)> getItems)
{
  std::cout << "开始执行Eclat算法..." << std::endl;
  
  std::cout << "步骤1/4: 正在收集所有项并建立TID列表..." << std::endl;
  std::unordered_map<std::string, std::vector<int>> tidLists;
  std::unordered_set<std::string> allItems;
  
  for (int tid = 0; tid < data.size(); tid++)
  {
    for (const auto &item : getItems(data[tid]))
    {
      if (item.empty())
        continue;
        
      allItems.insert(item);
      tidLists[item].push_back(tid);
    }
  }
  std::cout << "找到 " << allItems.size() << " 个不同的项" << std::endl;
  
  std::cout << "步骤2/4: 正在筛选满足最小支持度的项..." << std::endl;
  std::vector<std::pair<std::string, std::vector<int>>> frequentItems;
  int processedItems = 0;
  
  for (const auto &item : allItems)
  {
    int support = tidLists[item].size();
    if (support >= min_sup)
    {
      frequentItems.push_back({item, tidLists[item]});
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
  std::cout << "找到 " << frequentItems.size() << " 个项满足最小支持度 " << min_sup << std::endl;
  
  std::sort(frequentItems.begin(), frequentItems.end(), 
            [](const auto &a, const auto &b) {
              return a.second.size() > b.second.size();
            });
  
  std::cout << "步骤3/4: 正在递归挖掘频繁项集..." << std::endl;
  std::vector<std::vector<std::string>> result;
  
  std::function<void(std::vector<std::pair<std::string, std::vector<int>>>, std::vector<std::string>)> 
  mine = [&](std::vector<std::pair<std::string, std::vector<int>>> prefixTidLists, std::vector<std::string> prefix) {
    for (size_t i = 0; i < prefixTidLists.size(); i++)
    {
      std::vector<std::string> newPrefix = prefix;
      newPrefix.push_back(prefixTidLists[i].first);
      
      result.push_back(newPrefix);
      
      std::vector<std::pair<std::string, std::vector<int>>> newPrefixTidLists;
      
      for (size_t j = i + 1; j < prefixTidLists.size(); j++)
      {
        std::vector<int> intersection;
        std::set_intersection(
          prefixTidLists[i].second.begin(), prefixTidLists[i].second.end(),
          prefixTidLists[j].second.begin(), prefixTidLists[j].second.end(),
          std::back_inserter(intersection)
        );
        
        if (intersection.size() >= min_sup)
        {
          newPrefixTidLists.push_back({prefixTidLists[j].first, intersection});
        }
      }
      
      if (!newPrefixTidLists.empty())
      {
        mine(newPrefixTidLists, newPrefix);
      }
    }
  };
  
  mine(frequentItems, {});
  
  std::cout << "步骤4/4: 正在整理最终结果..." << std::endl;
  std::cout << "Eclat算法完成，共找到 " << result.size() << " 个频繁项集" << std::endl;
  
  return result;
}

#endif // ECLAT_H
