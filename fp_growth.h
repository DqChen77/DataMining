#include <vector>
#include <iostream>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <iomanip>
#include <memory>
#include "publication.h"


// FP树节点结构
struct FPNode
{
  std::string author;
  int count;
  std::shared_ptr<FPNode> parent;
  std::unordered_map<std::string, std::shared_ptr<FPNode>> children;
  std::shared_ptr<FPNode> nodeLink;

  FPNode(const std::string &a = "", int c = 0, std::shared_ptr<FPNode> p = nullptr)
      : author(a), count(c), parent(p), nodeLink(nullptr) {}
};

// FP树结构
class FPTree
{
public:
  std::shared_ptr<FPNode> root;
  std::unordered_map<std::string, std::shared_ptr<FPNode>> headerTable;

  FPTree() : root(std::make_shared<FPNode>()) {}

  // 向FP树中插入一条事务
  void insert(const std::vector<std::string> &transaction, int count)
  {
    std::shared_ptr<FPNode> currentNode = root;

    for (const auto &item : transaction)
    {
      if (currentNode->children.find(item) != currentNode->children.end())
      {
        // 如果当前节点已有该项的子节点，增加计数
        currentNode->children[item]->count += count;
      }
      else
      {
        // 创建新节点
        std::shared_ptr<FPNode> newNode = std::make_shared<FPNode>(item, count, currentNode);
        currentNode->children[item] = newNode;

        // 更新头表链接
        if (headerTable.find(item) != headerTable.end())
        {
          std::shared_ptr<FPNode> nodePtr = headerTable[item];
          while (nodePtr->nodeLink != nullptr)
          {
            nodePtr = nodePtr->nodeLink;
          }
          nodePtr->nodeLink = newNode;
        }
        else
        {
          headerTable[item] = newNode;
        }
      }
      currentNode = currentNode->children[item];
    }
  }

  // 检查树是否只包含单一路径
  bool hasSinglePath(std::shared_ptr<FPNode> node)
  {
    if (node->children.size() > 1)
    {
      return false;
    }
    if (node->children.empty())
    {
      return true;
    }
    // 递归检查唯一的子节点
    auto it = node->children.begin();
    return hasSinglePath(it->second);
  }
};

// 从条件模式基构建条件FP树
FPTree buildConditionalFPTree(const std::vector<std::pair<std::vector<std::string>, int>> &conditionalPatternBase, int minSup)
{
  // 计算条件模式基中每个项的支持度
  std::unordered_map<std::string, int> itemCount;
  for (const auto &pattern : conditionalPatternBase)
  {
    for (const auto &item : pattern.first)
    {
      itemCount[item] += pattern.second;
    }
  }

  // 移除不满足最小支持度的项
  std::unordered_map<std::string, int> frequentItems;
  for (const auto &pair : itemCount)
  {
    if (pair.second >= minSup)
    {
      frequentItems[pair.first] = pair.second;
    }
  }

  // 构建条件FP树
  FPTree conditionalTree;
  for (const auto &pattern : conditionalPatternBase)
  {
    std::vector<std::string> filteredPattern;
    for (const auto &item : pattern.first)
    {
      if (frequentItems.find(item) != frequentItems.end())
      {
        filteredPattern.push_back(item);
      }
    }

    // 按支持度排序
    std::sort(filteredPattern.begin(), filteredPattern.end(),
              [&frequentItems](const std::string &a, const std::string &b)
              {
                return frequentItems[a] > frequentItems[b];
              });

    if (!filteredPattern.empty())
    {
      conditionalTree.insert(filteredPattern, pattern.second);
    }
  }

  return conditionalTree;
}

// 从FP树中挖掘频繁项集
void mineTree(FPTree &tree, std::string suffix, std::vector<std::pair<std::vector<std::string>, int>> &frequentPatterns, int minSup)
{
  // 如果树只有一条路径，直接生成所有可能的组合
  if (tree.hasSinglePath(tree.root))
  {
    std::vector<std::pair<std::string, int>> path;
    std::shared_ptr<FPNode> node = tree.root;
    while (!node->children.empty())
    {
      auto it = node->children.begin();
      path.push_back({it->first, it->second->count});
      node = it->second;
    }

    // 生成所有可能的组合
    int n = path.size();
    for (int i = 1; i < (1 << n); i++)
    {
      std::vector<std::string> pattern;
      int minCount = INT_MAX;

      for (int j = 0; j < n; j++)
      {
        if (i & (1 << j))
        {
          pattern.push_back(path[j].first);
          minCount = std::min(minCount, path[j].second);
        }
      }

      if (!suffix.empty())
      {
        pattern.push_back(suffix);
      }

      frequentPatterns.push_back({pattern, minCount});
    }
  }
  else
  {
    // 对每个头表项，生成条件模式基和条件FP树
    for (auto it = tree.headerTable.begin(); it != tree.headerTable.end(); ++it)
    {
      std::string item = it->first;

      // 生成新的频繁项集
      std::vector<std::string> newPattern;
      newPattern.push_back(item);
      if (!suffix.empty())
      {
        newPattern.push_back(suffix);
      }

      // 计算支持度
      int support = 0;
      std::shared_ptr<FPNode> node = it->second;
      while (node != nullptr)
      {
        support += node->count;
        node = node->nodeLink;
      }

      frequentPatterns.push_back({newPattern, support});

      // 构建条件模式基
      std::vector<std::pair<std::vector<std::string>, int>> conditionalPatternBase;
      node = it->second;

      while (node != nullptr)
      {
        std::vector<std::string> path;
        std::shared_ptr<FPNode> parent = node->parent;

        // 从节点到根（不包括根）的路径
        while (parent != tree.root)
        {
          path.push_back(parent->author);
          parent = parent->parent;
        }

        if (!path.empty())
        {
          conditionalPatternBase.push_back({path, node->count});
        }

        node = node->nodeLink;
      }

      // 构建条件FP树并递归挖掘
      if (!conditionalPatternBase.empty())
      {
        FPTree conditionalTree = buildConditionalFPTree(conditionalPatternBase, minSup);
        mineTree(conditionalTree, item, frequentPatterns, minSup);
      }
    }
  }
}

// 主FP-Growth算法
template <typename T>
std::vector<std::vector<std::string>> FPGrowth(const std::vector<T> &data, int minSup, std::function<std::vector<std::string>(const T &)> getItems)
{
  std::cout << "开始执行FP-Growth算法..." << std::endl;

  // 步骤1: 扫描数据库，获取所有项及其支持度
  std::cout << "步骤1/4: 正在收集所有项及其支持度..." << std::endl;
  std::unordered_map<std::string, int> itemSupport;

  for (const auto &record : data)
  {
    for (const auto &item : getItems(record))
    {
      // 跳过空项
      if (item.empty())
        continue;
        
      itemSupport[item]++;
    }
  }

  // 过滤不满足最小支持度的项
  std::unordered_map<std::string, int> frequentItems;
  for (const auto &pair : itemSupport)
  {
    if (pair.second >= minSup)
    {
      frequentItems[pair.first] = pair.second;
    }
  }

  std::cout << "找到 " << frequentItems.size() << " 个项满足最小支持度 " << minSup << std::endl;

  // 步骤2: 构建FP树
  std::cout << "步骤2/4: 正在构建FP树..." << std::endl;
  FPTree tree;

  // 对每条记录，提取频繁项并按支持度排序
  int processedRecords = 0;
  for (const auto &record : data)
  {
    std::vector<std::pair<std::string, int>> items;

    for (const auto &item : getItems(record))
    {
      if (!item.empty() && frequentItems.find(item) != frequentItems.end())
      {
        items.push_back({item, frequentItems[item]});
      }
    }

    // 按支持度降序排序
    std::sort(items.begin(), items.end(),
              [](const std::pair<std::string, int> &a, const std::pair<std::string, int> &b)
              {
                return a.second > b.second;
              });

    // 提取排序后的项列表
    std::vector<std::string> transaction;
    for (const auto &item : items)
    {
      transaction.push_back(item.first);
    }

    // 插入FP树
    if (!transaction.empty())
    {
      tree.insert(transaction, 1);
    }

    processedRecords++;
    if (processedRecords % 100 == 0 || processedRecords == static_cast<int>(data.size()))
    {
      std::cout << "\r处理进度: " << std::fixed << std::setprecision(1)
                << (processedRecords * 100.0 / data.size()) << "% ("
                << processedRecords << "/" << data.size() << ")" << std::flush;
    }
  }
  std::cout << std::endl;

  // 步骤3: 挖掘频繁项集
  std::cout << "步骤3/4: 正在挖掘频繁项集..." << std::endl;
  std::vector<std::pair<std::vector<std::string>, int>> frequentPatterns;
  mineTree(tree, "", frequentPatterns, minSup);

  // 步骤4: 整理结果
  std::cout << "步骤4/4: 正在整理最终结果..." << std::endl;
  std::vector<std::vector<std::string>> result;
  for (const auto &pattern : frequentPatterns)
  {
    result.push_back(pattern.first);
  }

  std::cout << "FP-Growth算法完成，共找到 " << result.size() << " 个频繁项集" << std::endl;
  return result;
}
