#pragma once
#include "/home/pianochip/func.h"
#include "SplitTool.h"
#include <fstream>
#include <cctype>
#include <sstream>
#include <filesystem>

class DictProducer
{
public:
//函数成员
    DictProducer(const string& dir); //构造函数
    DictProducer(const string& dir, SplitTool* splitTool); //构造函数，中文专用
    void buildEnDict(); //创建英文词典
    void buildCnDict(); //创建中文词典
    void createIndex(); //创建词典索引
    void storeDict(const char * filepath); //将词典写入文件
    void storeIndex(const char* filepath); //将词典索引写入文件
    void showFiles()const; //查看文件路径，作为测试用
    void showDict()const; //查看词典，作为测试用
    void getFiles(); //获取文件的绝对路径
    void pushDict(const string & word); //存储某个单词
    int dictoffset = 0; //计算偏移
    ~DictProducer();

private:
//数据成员
    vector<string> _files; //语料文件的绝对路径集合
    vector<pair<string, int>> _dict; //词典
    SplitTool* _splitTool; //分词工具
    map<string, set<int>> _index; //词典索引
};

