#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include "../3rdpart/simhash/include/simhash/Simhasher.hpp" // 引入第三方 Simhash 库
#include "SplitToolJieba.h"

using namespace std;
using namespace simhash;
const static int TOPK_NUMBER = 20;

class WebPage
{
public:
    // 构造函数，需要传入整个 <doc> 字符串、停用词表以及 Simhash 对象
    WebPage(const string &doc, set<string> &stopWordList, Simhasher &simhasher, SplitToolJieba& splitTool);
    
    int getDocId() const { return _docId; }
    string getDoc() const { return _doc; }
    map<string, int>& getWordsMap() { return _wordsMap; }
    uint64_t getSimhash() const { return _simhashValue; }

    // 核心处理逻辑：拆解 XML -> 算 Simhash -> 算 _wordsMap
    void processDoc(const string &doc, set<string> &stopWordList, Simhasher &simhasher, SplitToolJieba& splitTool);

    // 友元函数，供外部快速比较与排序
    friend bool operator==(const WebPage &lhs, const WebPage &rhs);
    friend bool operator<(const WebPage &lhs, const WebPage &rhs);

private:
    string _doc;         // 包含 xml 标签在内的整篇文档
    int _docId;          // 文档 id
    string _docTitle;    // 文档标题
    string _docUrl;      // 文档 URL
    string _docContent;  // 文档正文内容
    string _docSummary;  // 文档摘要

    uint64_t _simhashValue;     // 【新增】网页的 Simhash 数字指纹
    map<string, int> _wordsMap; // 保存文档中剔除停用词后的词频
};