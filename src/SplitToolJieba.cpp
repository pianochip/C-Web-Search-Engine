    #include "../include/SplitToolJieba.h"

    // 定义词典路径（注意路径要和你的执行目录对应）
    const char* const DICT_PATH = "../3rdpart/cppjieba/dict/jieba.dict.utf8";
    const char* const HMM_PATH = "../3rdpart/cppjieba/dict/hmm_model.utf8";
    const char* const USER_DICT_PATH = "../3rdpart/cppjieba/dict/user.dict.utf8";
    const char* const IDF_PATH = "../3rdpart/cppjieba/dict/idf.utf8";
    const char* const STOP_WORD_PATH = "../3rdpart/cppjieba/dict/stop_words.utf8";

    // 构造函数：使用初始化列表来启动 Jieba 引擎
    SplitToolJieba::SplitToolJieba() 
        : _jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH) {}

    SplitToolJieba::~SplitToolJieba() {}

    vector<string> SplitToolJieba::cut(const string& sentence) {
        vector<string> words = {};
        _jieba.CutForSearch(sentence, words);
        return words;
    }