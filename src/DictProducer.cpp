#include "../include/DictProducer.h"

// 构造函数
DictProducer::DictProducer(const string& dir) {
    namespace fs = std::filesystem;

    // 1. 安全检查：如果路径根本不存在，直接报错并返回
    if (!fs::exists(dir)) {
        std::cerr << "路径不存在: " << dir << "\n";
        return;
    }

    // 2. 类型判断：如果是单个普通文件（例如 english.txt）
    if (fs::is_regular_file(dir)) {
        _files.push_back(dir);
    }
    // 3. 类型判断：如果是目录/文件夹（例如 art/）
    else if (fs::is_directory(dir)) {
        // 使用 directory_iterator 遍历该目录下的所有内容
        for (const auto& entry : fs::directory_iterator(dir)) {
            // 过滤掉可能存在的子文件夹，只提取普通文本文件
            if (entry.is_regular_file()) {
                // entry.path().string() 能将路径对象转换回标准的 std::string
                _files.push_back(entry.path().string());
            }
        }
    }
}

DictProducer::DictProducer(const string& dir, SplitTool* splitTool) {
    _splitTool = splitTool;
    // 为了代码简洁，给 std::filesystem 起一个别名 fs
    namespace fs = std::filesystem;

    // 1. 安全检查：如果路径根本不存在，直接报错并返回
    if (!fs::exists(dir)) {
        std::cerr << "路径不存在: " << dir << "\n";
        return;
    }

    // 2. 类型判断：如果是单个普通文件（例如 english.txt）
    if (fs::is_regular_file(dir)) {
        _files.push_back(dir);
    }
    // 3. 类型判断：如果是目录/文件夹（例如 art/）
    else if (fs::is_directory(dir)) {
        // 使用 directory_iterator 遍历该目录下的所有内容
        for (const auto& entry : fs::directory_iterator(dir)) {
            // 过滤掉可能存在的子文件夹，只提取普通文本文件
            if (entry.is_regular_file()) {
                // entry.path().string() 能将路径对象转换回标准的 std::string
                _files.push_back(entry.path().string());
            }
        }
    }
}

// 析构函数
DictProducer::~DictProducer() {
}

//创建英文词典
void DictProducer::buildEnDict() {
    map<string, int> word = {};//统计词频
    //1.循环处理所有英文语料
    for(size_t i=0; i<_files.size(); i++){
        ifstream en_corpus(_files[i]);
        if(!en_corpus.is_open()){
            cerr << _files[i] << "打开失败\n";
            continue;
        }
    //2.清洗
        string line = {};
        while(getline(en_corpus, line)){
            for(size_t j=0; j<line.size(); j++){
                //2.1 大写->小写
                if(line[j] >= 'A' && line[j] <= 'Z')
                    line[j] += 32;
                //2.2 标点和\r->空格
                if(!isalpha(line[j]))
                    line[j] = ' ';
            }
            //3.切割+统计词频
            stringstream ss(line);
            string temp;
            while(ss >> temp){// >>运算自动跳过单词中间的单个或连续空格
                word[temp]++;
            }
        }
    }
    //4.将存储单词及词频的map结构的内容存入vector
    for(auto i=word.begin(); i!=word.end(); i++){
        _dict.push_back({i->first, i->second});
    }
    return;
}

// 辅助函数：严格判断一个字符串是否完全由标准的中文汉字组成
bool isPureChinese(const string& word) {
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

// 创建中文词典
void DictProducer::buildCnDict() {
    // 1. 加载停用词表
    set<string> stopwords = {};
    ifstream stopfile("../data/yuliao/stop_words_zh.txt");
    if(!stopfile.is_open()){
        cerr << "stop_words_zh.txt" << "打开失败\n";
        return;
    }
    string stopword;
    while(getline(stopfile, stopword)) {
        stopwords.insert(stopword);
    }
    stopfile.close();

    map<string, int> word = {}; // 统计词频

    // 2. 循环处理每一个中文语料文件
    for(auto &file : _files){
        // 使用 ios::ate 模式打开，文件指针会直接停在文件末尾
        ifstream ifs(file, std::ios::ate);
        if(!ifs.good()){
            cerr << file << " 打开失败\n";
            continue;
        } 

        // 2.1 获取文件长度
        size_t length = ifs.tellg();
        // 将文件指针重新调回文件开头
        ifs.seekg(std::ios_base::beg);

        // 2.2 动态申请内存并一次性读取整篇文章
        // 注意：加了 () 会自动初始化为 0，防止字符串没有结束符 '\0'
        char *buff = new char[length + 1](); 
        ifs.read(buff, length);
        string txt(buff);
        
        // 释放动态内存！
        delete[] buff; 
        ifs.close(); // 提前关闭文件流

        // 3. 将整篇文章直接交给 Jieba 一次性切分完毕
        vector<string> temp = _splitTool->cut(txt);

        // 4. 遍历切分结果，过滤并统计词频
        for(auto &w : temp){
            if(!stopwords.count(w) && isPureChinese(w)){
                word[w]++; // 统计词频
            }
        }
    }

    // 5. 将存储单词及词频的 map 结构的内容存入 vector (_dict)
    for(auto &i : word){
        _dict.push_back({i.first, i.second});
    }
}

//创建词典索引
void DictProducer::createIndex(){
    for(size_t i = 0; i < _dict.size(); i++){
        string word = _dict[i].first;
        
        // 内层循环：注意第三个条件留空，我们在循环体内手动控制步进长度
        for(size_t j = 0; j < word.size(); ){
            size_t charLen = 1; // 默认当前字符占 1 个字节（英文）
            unsigned char c = word[j]; // 必须用无符号 char 来判断高位
            
            // 利用 UTF-8 编码高位特征，动态判断当前这个字占几个字节
            if((c & 0x80) == 0) {
                charLen = 1; // 0xxxxxxx：ASCII 英文字符
            } else if ((c & 0xE0) == 0xC0) {
                charLen = 2; // 110xxxxx：占 2 字节字符
            } else if ((c & 0xF0) == 0xE0) {
                charLen = 3; // 1110xxxx：占 3 字节字符（绝大多数标准中文汉字）
            } else if ((c & 0xF8) == 0xF0) {
                charLen = 4; // 11110xxx：占 4 字节字符（极少数生僻字或 Emoji）
            }
            
            // 核心修正：按算出的字节长度整体截取，此时得到的是合法的 string 类型
            string singleChar = word.substr(j, charLen);
            
            // 类型完全匹配，直接插入行号
            _index[singleChar].insert(i + dictoffset);
            
            // 让指针精确跳到下一个字的开头
            j += charLen;
        }
    }
}

//将词典写入文件
void DictProducer::storeDict(const char* filepath){
    //1.创建并打开要写入的文件
    ofstream ofs(filepath, std::ios::app);

    //2.检查文件是否成功创建和打开
    if(!ofs.is_open()){
        cerr << filepath << "文件创建/打开失败\n";
        return;
    }

    //3.遍历_dict，将数据写入文件
    for(size_t i=0; i<_dict.size(); i++){
        ofs << _dict[i].first << ' ' << _dict[i].second << "\n";
    }

    return;
}

//将词典索引写入文件
void DictProducer::storeIndex(const char* filepath){
    //1.创建并打开要写入的文件
    ofstream ofs(filepath, std::ios::app);

    //2.检查文件是否成功创建和打开
    if(!ofs.is_open()){
        cerr << filepath << "文件创建/打开失败\n";
        return;
    }

    //3.遍历_index,将数据写入文件
    for(auto i=_index.begin(); i!=_index.end(); i++){
        // 先把单字写进去，后面跟一个空格
        ofs << i->first << " "; 

        // 嵌套内层循环：遍历这个单字对应的 set<int>
        for(auto set_it = i->second.begin(); set_it != i->second.end(); set_it++){
            // 迭代器本质是指针，需要加 * 解引用才能拿到里面的整型行号
            ofs << *set_it << " "; 
        }

        // 这个字的所有行号写完后，最后补一个换行符
        ofs << "\n";
    }
}

