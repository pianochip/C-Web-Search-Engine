#pragma once
#include <vector>
#include <string>
#include <map>
#include <set>
using namespace std;

class Dictionary
{
public:
    static Dictionary* createInstance(); //静态函数
    void init (const string & dictpath, const string & indexpath); //通过词典文件路径初始化词典和词典索引
    vector<pair<string, int> > & getDict(); //获取词典
    map<string, set<int>> & getIndexTable(); //获取索引表
    string doQuery(string word); //执行查询

private:
    Dictionary() {}
    ~Dictionary() {}
    vector<pair<string,int>> _dict; //词典
    map<string,set<int>> _indexTable; //索引表
};

