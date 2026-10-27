#include "/home/pianochip/func.h"
#include "../include/KeyRecommander.h"


// 构造函数：接收用户的错别字
KeyRecommander::KeyRecommander(const string& query) : _queryWord(query) {
}

// 执行查询并打印结果
json KeyRecommander::execute() {
    // 1. 触发召回、算距离、入队列的完整流水线
    queryIndexTable();

    json result; 
    vector<string> candidates; //候选词数组

    // 2. 准备从优先级队列中提取 Top K 的结果（比如前 5 个）
    int k = 5;
    cout << "\n>>> 正在为 [" << _queryWord << "] 寻找推荐结果..." << "\n";

    // 3. 循环弹出队列顶端（最优）的元素并存入数组
    int rank = 1;
    while (!_resultQue.empty() && k > 0) {
        MyResult res = _resultQue.top(); // 获取堆顶元素
        _resultQue.pop(); // 弹出堆顶，让下一个最优元素浮上来

        cout << "Top " << rank++ << " -> " 
             << "候选词: " << res._word 
             << " | 编辑距离: " << res._dist 
             << " | 词频: " << res._freq << "\n";
        
        candidates.push_back(res._word);
        k--;
        
        //将候选词插入json中
        if (candidates.empty()) {
            result["status"] = "not_found";
            result["message"] = "未找到相关候选词";
        } else {
            result["status"] = "success";
            result["candidates"] = candidates; // nlohmann/json 会自动把 vector 转换为 JSON 数组
        }
    }
    return result;
}

// 计算mindist的辅助函数：将 UTF-8 字符串安全地切分为单字数组
vector<string> splitUTF8(const string& str) {
    vector<string> chars;
    for (size_t i = 0; i < str.length(); ) {
        size_t charLen = 1;
        unsigned char c = str[i];
        if ((c & 0x80) == 0) charLen = 1;
        else if ((c & 0xE0) == 0xC0) charLen = 2;
        else if ((c & 0xF0) == 0xE0) charLen = 3;
        else if ((c & 0xF8) == 0xF0) charLen = 4;
        
        chars.push_back(str.substr(i, charLen));
        i += charLen;
    }
    return chars;
}

// 计算最小编辑距离（支持中文 UTF-8）
int KeyRecommander::distance(const string & rhs) {
    // 1. 先把底层的字节字符串，转换为真实的“单字”数组
    vector<string> leftChars = splitUTF8(_queryWord);
    vector<string> rightChars = splitUTF8(rhs);

    // 2. 获取真正的字符长度
    int leftsize = leftChars.size();
    int rightsize = rightChars.size();

    // 3. 构建 DP 表
    vector<vector<int>> mindist(leftsize + 1, vector<int>(rightsize + 1, 0));

    // 4. 初始化边界（注意循环条件是 <= 真实长度）
    for(int i = 1; i <= leftsize; i++) {
        mindist[i][0] = i;
    }
    for(int j = 1; j <= rightsize; j++) {
        mindist[0][j] = j;
    }

    // 5. 动态规划状态转移
    for(int i = 1; i <= leftsize; i++){
        for(int j = 1; j <= rightsize; j++){
            // 比较的是单字数组中的元素
            if(leftChars[i-1] == rightChars[j-1]){
                mindist[i][j] = mindist[i-1][j-1];
            } else {
                mindist[i][j] = min(mindist[i-1][j-1], min(mindist[i-1][j], mindist[i][j-1])) + 1;
            }
        }
    }
    
    return mindist[leftsize][rightsize];
}

//查询索引
void KeyRecommander::queryIndexTable() {
    //创建字典对象,获取词典索引
    Dictionary* dictInst = Dictionary::createInstance();
    const map<string, set<int>>& _indexTable = dictInst->getIndexTable();
    //将待查询的词分成一个个单字
    vector<string> query = splitUTF8(_queryWord);
    //查询每个单字的索引并记录
    set<int> iset;
    for(size_t i=0; i<query.size(); i++){
        auto it = _indexTable.find(query[i]);
        if (it != _indexTable.end()) {
            iset.insert(it->second.begin(), it->second.end());
        }
    }
    statistic(iset);
}

//根据收集到的行号去词典里提取单词,再计算编辑距离后传入优先级队列
void KeyRecommander::statistic(set<int> & iset) {
    //创建字典对象,获取词典
    Dictionary* dictInst = Dictionary::createInstance();
    const vector<pair<string, int>>& _dict = dictInst->getDict();
    
    //根据iset的内容去词典里收集单词
    // 直接在一个循环内完成所有动作
    for(auto i = iset.begin(); i != iset.end(); i++){
        // 1. 同时提取单词和词频
        string candidateWord = _dict[*i].first;
        int freq = _dict[*i].second;

        // 2. 用一个整型变量接收计算出来的编辑距离
        int dist = distance(candidateWord);

        // 3. 将这三个数据打包进结果结构体
        MyResult res;
        res._word = candidateWord;
        res._freq = freq;
        res._dist = dist;

        // 4. 塞入优先级队列，队列底层会根据 MyCompare 仿函数自动完成 Top K 排序
        _resultQue.push(res);
    }

}