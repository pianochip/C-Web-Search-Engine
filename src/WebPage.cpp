#include "/home/pianochip/func.h"
#include "../include/WebPage.h"

// 构造函数：直接把传进来的参数原封不动地丢给 processDoc 去处理
WebPage::WebPage(const string &doc, set<string> &stopWordList, Simhasher &simhasher, SplitToolJieba& splitTool) {
    processDoc(doc, stopWordList, simhasher, splitTool);
}

inline bool isPureChinese(const string& word) {
    if (word.empty()) return false;
    for (size_t i = 0; i < word.length(); ) {
        unsigned char c = word[i]; 
        if (c >= 0xE4 && c <= 0xE9) {
            i += 3; 
        } else {
            return false; 
        }
    }
    return true;
}

void WebPage::processDoc(const string &doc, set<string> &stopWordList, Simhasher &simhasher, SplitToolJieba& splitTool){
    //读取文档内容
    _doc = doc;
        //1.读取docid
    size_t start = doc.find("<docid>") + strlen("<docid>");
    size_t end = doc.find("</docid>");
    _docId = stoi(doc.substr(start, end - start));
        //2.读取doctitle
    start = doc.find("<title>") + strlen("<title>");
    end = doc.find("</title>");
    _docTitle = doc.substr(start, end - start);
        //3.读取docurl
    start = doc.find("<url>") + strlen("<url>");
    end = doc.find("</url");
    _docUrl = doc.substr(start, end - start);
        //4.读取doccontent
    start = doc.find("<content>") + strlen("<content>");
    end = doc.find("</content>");
    _docContent = doc.substr(start, end - start);

    //计算content的simhash值
    simhasher.make(_docContent, 20, _simhashValue);

    //分词
    vector<string> temp = splitTool.cut(_docContent);

    //统计词频
    for(auto &w : temp){
        if(!stopWordList.count(w) && isPureChinese(w)){
            _wordsMap[w]++; // 统计词频
        }
    }
}

// 比较两篇网页是否雷同（调用 Simhash 库静态方法，海明距离 <= 3 即判定为相同）
bool operator==(const WebPage &lhs, const WebPage &rhs) {
    return Simhasher::isEqual(lhs._simhashValue, rhs._simhashValue, 3);
}

// 供去重或排序时使用，按 docid 升序排列
bool operator<(const WebPage &lhs, const WebPage &rhs) {
    return lhs._docId < rhs._docId;
}