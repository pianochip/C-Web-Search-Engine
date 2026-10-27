#include "/home/pianochip/func.h"
#include "../include/WebPageQuery.h"
using json = nlohmann::json;

// 执行查询,返回结果 (提供给 SearchEngineServer 调用)
string WebPageQuery::doQuery(const string & str) {
    vector<pair<int, vector<double>>> resultVec;
    //1.使用 Jieba 分词工具对用户输入的原始字符串 str 进行切分，得到查询词数组 queryWords。
    vector<string> queryWords =_jieba.cut(str);

    //2.将 queryWords 传入getQueryWordsWeightVector，获取查询词的归一化权重向量X
    vector<double> X = getQueryWordsWeightVector(queryWords);

    //3.调用 executeQuery 获取同时包含所有搜索词的候选网页及其特征向量Y
    executeQuery(queryWords, resultVec);

    //4.计算余弦相似度, cosθ = (X*Y) / (|X|*|Y|)
    vector<pair<int ,double>> cosθs;
    vector<double> Y;
    for(size_t i=0; i<resultVec.size(); i++) {
        Y = resultVec[i].second;
        double cosθ = 0;
        for(size_t j=0; j<resultVec[i].second.size(); j++){
            cosθ += X[j] * Y[j];
        }
        cosθs.push_back({resultVec[i].first, cosθ});
    }

    //5.将网页按计算出的余弦相似度分数进行打分，分数越接近 1 越相似，需将其按降序（从大到小）重新排列
    sort(cosθs.begin(), cosθs.end(), [](const pair<int, double>& a, const pair<int, double>& b) {
        return a.second > b.second;
    });

    //6.抽取排好序的网页 docid 组成一个新的 vector<int> 数组，连同 queryWords 一起传给 createJson，最后直接 return 这个 JSON 字符串
    vector<int> docIdVec;
    for(size_t i=0; i<cosθs.size(); i++){
        docIdVec.push_back(cosθs[i].first);
    }
    string res = createJson(docIdVec, queryWords);

    //返回结果
    return res;
}

// 1. 加载库文件 (读取 final_offset.dat 和 invertindex.dat)
void WebPageQuery::loadLibrary() {
    //1.从磁盘读取文件
    ifstream offsetFs("../data/final_offset.dat");
    ifstream invertFs("../data/invertindex.dat");
    if(!offsetFs.is_open() || !invertFs.is_open()){
        cerr << "文件打开失败\n";
        return;
    }

    //2.将偏移库存到_offsetLib<docid, <start, length>>中
    string line;
    int docid, start, length;
    while(getline(offsetFs, line)){
        stringstream ss(line);
        ss >> docid >> start >> length;
        _offsetLib[docid] = {start, length};
    }

    //3.将索引表存到_invertIndexTable<string, set<pair<int, double>>>，<单词, 包含该词的网页及权重集合>
    while(getline(invertFs, line)){
        stringstream ss(line);
        string word;
        ss >> word; // 每一行的第一个肯定是单词

        int docid;
        double w;
        // 循环提取该单词后面的所有 <docid, 权重> 对
        while(ss >> docid >> w){
            _invertIndexTable[word].insert({docid, w});
        }
    }

    return;
}

// 2. 计算查询词的权重值 (将用户输入的句子分词，并计算 TF-IDF 基准向量)
vector<double> WebPageQuery::getQueryWordsWeightVector(vector<string> & queryWords) {
    //获取文章总数
    size_t docNum = _offsetLib.size();

    //计算查询词在数组里的出现次数TF
    map<string, int> freq;
    for (size_t i = 0; i < queryWords.size(); i++) {
        freq[queryWords[i]]++; // 如果键不存在会自动创建并加1，如果存在直接加1
    }

    // 遍历 map 计算 DF、IDF、初始权重 w
    double weightSum = 0; 
    vector<double> res;
    
    // 清空原本带有重复词、顺序未知的输入数组
    queryWords.clear(); 

    for(auto i = freq.begin(); i != freq.end(); i++){
        string word = i->first;
        // 防御：如果词完全不在词库里，DF 为 0
        int DF = _invertIndexTable.count(word) ? _invertIndexTable[word].size() : 0;
        
        double IDF = log2(static_cast<double>(docNum) / (DF + 1));
        double w = i->second * IDF; // tf * idf
        
        weightSum += pow(w, 2);
        
        // 同步存入词和权重，保证后续算出 Base 向量与特征向量 Y 一致
        queryWords.push_back(word); 
        res.push_back(w);
    }

    // 权重归一化
    if (weightSum > 0) {
        double normalizer = sqrt(weightSum);
        for(size_t i = 0; i < res.size(); i++){
            res[i] /= normalizer;
        }
    }

    return res;
}

// 3. 执行查询 (通过倒排索引找网页，并使用“余弦相似度”算法给网页打分排序)
bool WebPageQuery::executeQuery(const vector<string> & queryWords, 
    vector<pair<int, vector<double>>> & resultVec) {
    
    if (queryWords.empty()) return false;
    size_t wordsNum = queryWords.size();
    //1.检查queryWords里的所有单词是否都存在倒排索引
    for(size_t i=0; i<wordsNum; i++){
        if(_invertIndexTable.count(queryWords[i]) == 0){
            cerr << "该查询词没有对应的倒排索引\n";
            return false;
        }
    }

    //2.提取querWords[0]在_invertIndexTable里的对应的所有docid。存入vector<int> commonDocIds数组里
    vector<int> commonDocIds;
    set<pair<int, double>>& set = _invertIndexTable[queryWords[0]];
    for(auto i=set.begin(); i!=set.end(); i++){
        commonDocIds.push_back(i->first);
    }

    //3.用第一个单词的commonDocIds数组依此和其余querWords[i]的docid比较，剔除没有的
    if(wordsNum > 1){
        for(size_t i=1; i<wordsNum; i++){
            vector<int> tempDocIds;
            auto& currentSet = _invertIndexTable[queryWords[i]];

            for(size_t j=0; j<commonDocIds.size(); j++) {
                auto it = currentSet.lower_bound({commonDocIds[j], 0.0}); //检查commonDocIds[j]是否在其他单词的docid存在
                if(it != currentSet.end() && it->first == commonDocIds[j]){
                    //记录docid的交集
                    tempDocIds.push_back(it->first);
                }
            }
            commonDocIds = tempDocIds;
            if(commonDocIds.empty()){
                return false;
            }
        }
    }

    //4.提取commonDocIds里的网页对应查询词的权重，组成特征向量，然后存入resultVec
    for(size_t j = 0; j < commonDocIds.size(); j++) {
        int docid = commonDocIds[j];
        vector<double> featureY; 
        
        // 按照 queryWords 的顺序，去倒排索引里提取该网页对应的权重
        for(size_t i = 0; i < wordsNum; i++){
            auto& currentSet = _invertIndexTable[queryWords[i]];
            auto it = currentSet.lower_bound({docid, 0.0});
            featureY.push_back(it->second); 
        }
        
        // 将组装好的特征向量存入结果集
        resultVec.push_back({docid, featureY});
    }

    return true;
}

// 4. 封装结果 (提取网页标题、URL、自动生成摘要，并封装成 JSON 字符串)
string WebPageQuery::createJson(vector<int> & docIdVec, 
    const vector<string> & queryWords) {
    
    //1.创建一个空的 JSON 数组对象，并以读取模式打开离线生成的网页库文件
    nlohmann::json responseJson = nlohmann::json::array();
    ifstream pageFile("../data/final_ripepage.dat", ios::in);
    if(!pageFile.is_open()){
        cerr << "文件打开失败\n";
        return returnNoAnswer();
    }

    //2.根据传入的 docIdVec 逐一处理每个 docid。
    for(size_t i=0; i<docIdVec.size(); i++){
        //3.通过 _offsetLib 查到当前 docid 在磁盘文件中的起始偏移量和文本长度。获取这篇网页的完整 XML 格式原文
        int docid = docIdVec[i];
        int start = _offsetLib[docid].first;
        int length = _offsetLib[docid].second;
            //获取原文
        pageFile.seekg(start, ios::beg);
        string docStr(length, '\0');
        pageFile.read(&docStr[0], length);

        //4.读取到的单篇网页原文中，查找 <title>、<url> 和 <content> 等标签的位置，提取出包裹在其中的纯文本内容
        string title, url, content;
        size_t titlePos = docStr.find("<title>");
        size_t titleEnd = docStr.find("</title>");
        if (titlePos != string::npos && titleEnd != string::npos) {
            title = docStr.substr(titlePos + 7, titleEnd - titlePos - 7);
        }

        size_t urlPos = docStr.find("<url>");
        size_t urlEnd = docStr.find("</url>");
        if (urlPos != string::npos && urlEnd != string::npos) {
            url = docStr.substr(urlPos + 5, urlEnd - urlPos - 5);
        }
        
        size_t contentPos = docStr.find("<content>");
        size_t contentEnd = docStr.find("</content>");
        if (contentPos != string::npos && contentEnd != string::npos) {
            content = docStr.substr(contentPos + 9, contentEnd - contentPos - 9);
        }

        //5.编写摘要，遍历传入的 queryWords，在提取出的正文（content）中寻找查询词首次出现的位置。以该匹配点为基准，向前后截取一段适中长度的字符片段，并在末尾拼接省略号
        string summary;
        size_t summaryPos = string::npos;
            //寻找第一个在正文中出现的查询词
        for (const string& word : queryWords) {
            summaryPos = content.find(word);
            if (summaryPos != string::npos) {
                break;
            }
         }

        if (summaryPos != string::npos) {
            // 为了上下文连贯，摘要从查询词出现的位置往前退一点开始截取
            size_t startPos = (summaryPos > 30) ? summaryPos - 30 : 0;
            // 截取大约 150 个字节作为摘要，并加上省略号
            summary = content.substr(startPos, 150) + "...";
        } else {
            // 如果解析偏差导致没找到，默认截取开头
            summary = content.substr(0, 150) + "...";
        }

        //6.将提取好的标题、URL 和生成的摘要组装成一个独立的 JSON，追加到最开始创建的 JSON 数组中
        json node;
        node["title"] = title;
        node["url"] = url;
        node["summary"] = summary;
        responseJson.push_back(node);
    }
    pageFile.close();
    
    // 如果没有任何结果，返回无结果 JSON
    if(responseJson.empty()){
        return returnNoAnswer();
    }
    
    return responseJson.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace); // 将 JSON 对象序列化为字符串返回
}

// 5. 查询失败时的备用返回方法
string WebPageQuery::returnNoAnswer() {
    json responseJson = json::array();
    json node;
    node["title"] = "未找到相关结果";
    node["url"] = "";
    node["summary"] = "抱歉，没有找到包含您输入关键词的网页，请尝试更换搜索词。";
    responseJson.push_back(node);
    return responseJson.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
}