#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <pugixml.hpp>
#include <algorithm>
#include <random>
#include <unordered_map>
#include "publication.h"
#include "dblp_reader.h"

DBLPReader::DBLPReader(const std::string& filename) : filename_(filename) {}

std::vector<Publication> DBLPReader::readPublications() {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filename_.c_str());

    if (!result) {
        std::cerr << "无法加载XML文件: " << result.description() << std::endl;
        return std::vector<Publication>();
    }

    // 获取根节点
    pugi::xml_node root = doc.child("dblp");
    
    std::vector<Publication> allPublications;
    
    // 遍历所有出版物节点
    for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling()) {
        Publication pub;
        pub.key = node.attribute("key").value();
        
        // 读取标题
        pugi::xml_node title_node = node.child("title");
        if (title_node) {
            pub.title = title_node.child_value();
        }
        
        // 读取作者
        for (pugi::xml_node author_node = node.child("author"); 
             author_node; 
             author_node = author_node.next_sibling("author")) {
            pub.authors.push_back(author_node.child_value());
        }
        
        // 读取年份
        pugi::xml_node year_node = node.child("year");
        if (year_node) {
            pub.year = year_node.child_value();
        }
        
        // 读取会议/期刊名称
        pugi::xml_node venue_node = node.child("journal");
        if (!venue_node) {
            venue_node = node.child("booktitle");
        }
        if (!venue_node) {
            venue_node = node.child("proceedings");
        }
        if (venue_node) {
            pub.venue = venue_node.child_value();
        }
        
        // 收集出版物
        allPublications.push_back(pub);
    }
    
    // 随机选择10,000条记录并保存为XML
    // saveRandomPublicationsToXML(allPublications, 10000, "data/random_dblp_sample.xml");
    return allPublications;
}

void DBLPReader::processPublication(const Publication& pub) {
    // 在这里处理每篇出版物
    // 例如：打印信息、存储到数据库等
    std::cout << "Key: " << pub.key << std::endl;
    std::cout << "Title: " << pub.title << std::endl;
    std::cout << "Authors: ";
    for (const auto& author : pub.authors) {
        std::cout << author << ", ";
    }
    std::cout << std::endl;
    std::cout << "Year: " << pub.year << std::endl;
    std::cout << "Venue: " << pub.venue << std::endl;
    std::cout << "------------------------" << std::endl;
}

void DBLPReader::saveRandomPublicationsToXML(const std::vector<Publication>& publications, int count, const std::string& outputFilename) {
    // 如果总数少于要求的数量，使用全部
    size_t sampleSize = std::min(static_cast<size_t>(count), publications.size());
    
    // 随机选择sampleSize条记录
    std::vector<Publication> sampledPublications;
    if (sampleSize < publications.size()) {
        std::vector<size_t> indices(publications.size());
        for (size_t i = 0; i < indices.size(); i++) {
            indices[i] = i;
        }
        
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(indices.begin(), indices.end(), g);
        
        for (size_t i = 0; i < sampleSize; i++) {
            sampledPublications.push_back(publications[indices[i]]);
        }
    } else {
        sampledPublications = publications;
    }
    
    // 创建新的XML文档
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("dblp");
    
    // 添加选中的出版物到XML
    for (const auto& pub : sampledPublications) {
        // 确定出版物类型（假设从key中提取）
        std::string pubType = "article"; // 默认类型
        if (pub.key.find("conf") != std::string::npos) {
            pubType = "inproceedings";
        } else if (pub.key.find("journals") != std::string::npos) {
            pubType = "article";
        }
        
        pugi::xml_node pubNode = root.append_child(pubType.c_str());
        pubNode.append_attribute("key") = pub.key.c_str();
        
        // 添加标题
        if (!pub.title.empty()) {
            pubNode.append_child("title").text().set(pub.title.c_str());
        }
        
        // 添加作者
        for (const auto& author : pub.authors) {
            pubNode.append_child("author").text().set(author.c_str());
        }
        
        // 添加年份
        if (!pub.year.empty()) {
            pubNode.append_child("year").text().set(pub.year.c_str());
        }
        
        // 添加会议/期刊名称
        if (!pub.venue.empty()) {
            if (pubType == "article") {
                pubNode.append_child("journal").text().set(pub.venue.c_str());
            } else {
                pubNode.append_child("booktitle").text().set(pub.venue.c_str());
            }
        }
    }
    
    // 保存XML文档
    bool saveResult = doc.save_file(outputFilename.c_str());
    if (saveResult) {
        std::cout << "成功保存 " << sampleSize << " 条随机出版物记录到 " << outputFilename << std::endl;
    } else {
        std::cerr << "保存XML文件失败: " << outputFilename << std::endl;
    }
}