#include "/home/pianochip/func.h"
#include "../include/Dictionary.h"
#include <fstream>
#include <sstream>

Dictionary* Dictionary::createInstance(){//对象实例化
    static Dictionary instance;
    return &instance;
}

//初始化词典
void Dictionary::init (const string & dictpath, const string & indexpath){
    // ========== 加载词典 dict.dat ==========
    ifstream dictfile(dictpath);
    if(!dictfile.is_open()){
        cerr << dictpath << "文件打开失败\n";
        return;
    }

    string word;
    int freq;
    while(dictfile >> word >> freq){
        _dict.push_back({word, freq});
    }
    dictfile.close();

    // ========== 加载索引 dictIndex.dat==========
    ifstream indexfile(indexpath);
    if(!indexfile.is_open()){
        cerr << indexpath << "文件打开失败\n";
        return;
    }

    string line;
    while(getline(indexfile, line)){//循环读取每一行
        stringstream ss(line);
        //读取单字
        string keystring;
        ss >> keystring;

        //读取索引
        int lineno;
        set<int> tempset;
        while(ss >> lineno){
            tempset.insert(lineno);
        }

        _indexTable[keystring] = tempset;
    }
    indexfile.close();
}

//获取词典
vector<pair<string, int> > & Dictionary::getDict(){
    return _dict;
}

//获取词典索引表
map<string, set<int> > & Dictionary::getIndexTable(){
    return _indexTable;
}

//查询
string doQuery(string word){
    return {};
}