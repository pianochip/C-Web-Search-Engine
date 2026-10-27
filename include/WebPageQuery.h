#pragma once
#include "WebPage.h"
#include "SplitToolJieba.h"
#include "nlohmann/json.hpp"
#include <vector>
#include <unordered_map>
#include <set>
#include <string>
#include <cmath>
#include <algorithm>
using namespace std;

class WebPageQuery {
public:
    WebPageQuery() {};
    
    // 执行查询,返回结果 (提供给 SearchEngineServer 调用)
    string doQuery(const string & str);

    string returnNoAnswer();

    // 1. 加载库文件 (读取 final_offset.dat 和 invertindex.dat)
    void loadLibrary();

private:
    // 2. 计算查询词的权重值 (将用户输入的句子分词，并计算 TF-IDF 基准向量)
    vector<double> getQueryWordsWeightVector(vector<string> & queryWords);
    
    // 3. 执行查询 (通过倒排索引找网页，并使用“余弦相似度”算法给网页打分排序)
    bool executeQuery(const vector<string> & queryWords, 
                      vector<pair<int, vector<double>>> & resultVec);
    
    // 4. 封装结果 (提取网页标题、URL、自动生成摘要，并封装成 JSON 字符串)
    string createJson(vector<int> & docIdVec, 
                           const vector<string> & queryWords);
                           
    

private:
    SplitToolJieba _jieba; // Jieba 分词库对象
    
    //存放网页库
    unordered_map<int, WebPage> _pageLib; 
    
    // 存放偏移库：<docid, <start, length>>
    unordered_map<int, pair<int, int>> _offsetLib; 
    
    // 存放倒排索引表，<单词, 包含该词的网页及权重集合>
    unordered_map<string, set<pair<int, double>>> _invertIndexTable; 
    

};