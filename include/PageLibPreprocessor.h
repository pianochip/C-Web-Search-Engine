#pragma once
#include "WebPage.h"
#include <vector>
#include <unordered_map>
#include <cmath>

class PageLibPreprocessor {
public:
    PageLibPreprocessor();

    // 供外部 main 函数调用的唯一公共接口
    void doProcess();

    // 1. 根据 offset.dat 读取 ripepage.dat，实例化 WebPage 并推入 _pageLib
    void readInfoFromFile();
    
    // 2. 核心查重：遍历 _pageLib，利用 WebPage::operator== 进行比对剔除
    void cutRedundantPages();
    
    // 3. 核心算法：基于 TF 词频与 IDF 逆文档频率，计算每个词的权重，构建倒排索引
    void buildInvertIndexTable();
    
    // 4. 落盘：生成去重后的 newpage.dat、newoffset.dat 以及 invertindex.dat
    void storeOnDisk();

private:
    vector<WebPage> _pageLib; //存储所有WebPage
    unordered_map<string, vector<pair<int, double>>> _invertIndexTable;// 倒排索引表：<词, vector<pair<docid, 权重w>>>
    unordered_map<string, int> _dfMap; // 记录每个词在多少篇文章中出现过
    
    SplitToolJieba _splitTool;
    Simhasher _simhasher;
    set<string> _stopWordList;
};
