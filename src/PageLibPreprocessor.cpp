#include "/home/pianochip/func.h"
#include "../include/PageLibPreprocessor.h"

void PageLibPreprocessor::doProcess() {
    readInfoFromFile();
    cutRedundantPages();
    buildInvertIndexTable();
    storeOnDisk();
}

//构造函数,加载停用词表
PageLibPreprocessor::PageLibPreprocessor() 
: _simhasher("../3rdpart/simhash/submodules/cppjieba/dict/jieba.dict.utf8", 
             "../3rdpart/simhash/submodules/cppjieba/dict/hmm_model.utf8", 
             "../3rdpart/simhash/submodules/cppjieba/dict/idf.utf8", 
             "../3rdpart/simhash/submodules/cppjieba/dict/stop_words.utf8") 
{
    // 1. 打开停用词表文件
    ifstream ifs("../3rdpart/simhash/submodules/cppjieba/dict/stop_words.utf8");
    if (!ifs.is_open()) {
        cerr << "停用词表打开失败！路径错误" << endl;
        return;
    }

    // 2. 按行读取并存入 set 集合
    string word;
    while (getline(ifs, word)) {
        // 消除 Windows 格式文件可能带来的回车符 \r 隐患
        if (!word.empty() && word.back() == '\r') {
            word.pop_back();
        }
        
        // 插入到 _stopWordList 集合中
        _stopWordList.insert(word);
    }
    
    ifs.close();
    cout << ">>> 成功加载停用词：" << _stopWordList.size() << " 个" << "\n";
}

// 1. 根据 offset.dat 读取 newripepage.dat，实例化 WebPage 并推入 _pageLib
void PageLibPreprocessor::readInfoFromFile() {
    ifstream fs1("../data/newoffset.dat");
    ifstream fs2("../data/newripepage.dat");
    if (!fs1.is_open() || !fs2.is_open()) {
        cerr << "网页库或偏移库打开失败！" << "\n";
        return;
    }

    string offsetline;
    while(getline(fs1, offsetline)){
        //分割每行的字符串
        stringstream ss(offsetline);
        //接收每行的数据
        string num;
        ss >> num; //提取docid
        //int docid = stoi(num);
        ss >> num; //提取doc的开头位置
        int start = stoi(num);
        ss >> num; //提取doc的长度
        int length = stoi(num);

        //根据提取出来的doc开头位置和长度读取内容
        fs2.seekg(start, std::ios::beg); //定位到开始位置

        string docStr; //读取内容
        docStr.resize(length);
        fs2.read(&docStr[0], length);

        //构建webpage对象
        WebPage webPage(docStr, _stopWordList, _simhasher, _splitTool);
        
        //存到网页库容器中
        _pageLib.push_back(webPage);
    }
    fs1.close();
    fs2.close();
}

// 2. 查重：遍历 _pageLib，利用 WebPage::operator== 进行比对剔除
void PageLibPreprocessor::cutRedundantPages() {
    // 存放去重后的网页
    vector<WebPage> uniquePage;

    for(size_t i = 0; i < _pageLib.size(); i++){
        bool same = false;
        for(size_t j = 0; j < uniquePage.size(); j++){
            if(_pageLib[i] == uniquePage[j]){
                same = true;
                break;
            }
        }
        if(same == false) {
            uniquePage.push_back(_pageLib[i]);
        }
    }
    //更新_pageLib
    _pageLib = uniquePage;
}

// 3. 基于 TF 词频与 IDF 逆文档频率，计算每个词的权重，构建倒排索引
void PageLibPreprocessor::buildInvertIndexTable() {
    size_t docNum = _pageLib.size();
    //1.统计所有词的文档频率（DF）
    for(size_t i=0; i<docNum; i++) {
        for(auto j=_pageLib[i].getWordsMap().begin(); j != _pageLib[i].getWordsMap().end(); j++){
            _dfMap[j->first] ++; //计算DF
        }
    }

    //根据1的结果，计算IDF, w。构建倒排索引表
    for(size_t i=0; i<docNum; i++) {
        unordered_map<string, double> weightMap; //存当前文章各个词的初始权重
        double weightSum = 0; //累加权重的平方和
        for(auto j=_pageLib[i].getWordsMap().begin(); j != _pageLib[i].getWordsMap().end(); j++){
            //计算 IDF：利用公式 double idf = log2(N / (DF + 1))
            double IDF = log2(static_cast<double>(docNum) / (_dfMap[j->first] + 1));
            //计算初始权重w：利用公式double w = TF * idf;
            double w = j->second * IDF;
            //存储每个词的初始权重w以及累加权重的平方和
            weightMap[j->first] = w;
            weightSum += pow(w, 2);
        }
        
        //计算当前文章的归一化分母
        double normalizer = sqrt(weightSum);
        //构建倒排索引表
        for(auto j=weightMap.begin(); j!=weightMap.end(); j++){
            //计算单篇文章里的每个单词的最终权重
            double finalWeight = j->second / normalizer;
            _invertIndexTable[j->first].push_back(make_pair(_pageLib[i].getDocId(), finalWeight));
        }

        // 4. 落盘：生成去重后的 newpage.dat、newoffset.dat 以及 invertindex.dat
        void storeOnDisk();
    }
}

// 4. 落盘：生成去重后的 newpage.dat、newoffset.dat 以及 invertindex.dat
void PageLibPreprocessor::storeOnDisk() {
    // 1. 打开三个目标文件
    ofstream ofsRipe("../data/final_ripepage.dat");
    ofstream ofsOffset("../data/final_offset.dat");
    ofstream ofsIndex("../data/invertindex.dat");

    if(!ofsRipe.is_open() || !ofsOffset.is_open() || !ofsIndex.is_open()) {
        cerr << "目标存储文件打开失败！\n";
        return;
    }

    // 2. 将去重后的网页库和偏移库写入磁盘
    for(size_t i = 0; i < _pageLib.size(); i++) {
        int docId = _pageLib[i].getDocId();
        string docStr = _pageLib[i].getDoc(); // 获取带有 <doc> 标签的完整原文
        
        // 记录写入前的偏移量 start
        int start = ofsRipe.tellp();
        int length = docStr.size();
        
        // 写入 offset.dat
        ofsOffset << docId << " " << start << " " << length << "\n";
        
        // 写入 ripepage.dat
        ofsRipe << docStr;
    }

    // 3. 将倒排索引表写入磁盘
    for(auto it = _invertIndexTable.begin(); it != _invertIndexTable.end(); it++) {
        string word = it->first;
        ofsIndex << word; // 先写入单词
        
        // 遍历包含这个词的所有文章及其权重
        vector<pair<int, double>>& docList = it->second;
        for(size_t i = 0; i < docList.size(); i++) {
            ofsIndex << " " << docList[i].first << " " << docList[i].second;
        }
        ofsIndex << "\n"; // 这个词的数据写完，换行
    }

    ofsRipe.close();
    ofsOffset.close();
    ofsIndex.close();
    
    cout << ">>> 已生成 final_ripepage.dat, final_offset.dat, invertindex.dat\n";
}