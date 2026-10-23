#pragma once
#include "Dictionary.h" 
#include "nlohmann/json.hpp"
#include <vector>
#include <string>
#include <set>
#include <queue>
using namespace std;
using json = nlohmann::json;

class MyResult{
public:
    string _word; // 候选词
    int _freq;    // 词频
    int _dist;    // 最小编辑距离
};

//定义优先级队列的比较规则
struct MyCompare{
    bool operator()(const MyResult& lhs, const MyResult& rhs) {
        // 第一维度：优先比较最小编辑距离，距离越小越靠前
        if (lhs._dist != rhs._dist) {
            return lhs._dist > rhs._dist; 
        }
        // 第二维度：距离相同，比较词频，词频越大越靠前
        if (lhs._freq != rhs._freq) {
            return lhs._freq < rhs._freq;
        }
        // 第三维度：词频也相同，按字母表顺序排列
        return lhs._word > rhs._word;
    }
};

class KeyRecommander
{
public:
    //KeyRecommander(string&query, const TcpConnectionPtr&);
    KeyRecommander(const string& query);
    KeyRecommander() {}
    ~KeyRecommander() {}
    json execute(); //执行查询

private:
    //数据成员
    string _queryWord; //等查询的单词
    //TcpConnectionPtr _conn; //与客户端进行连接的文件描述符
    priority_queue<MyResult, vector<MyResult>, MyCompare> _resultQue; //保存候选结果集的优先级队列
    //函数成员
    void queryIndexTable(); //查询索引
    void statistic(set<int> & iset); //根据收集到的行号去词典里提取单词,再计算编辑距离后传入优先级队列
    int distance(const string & rhs); //计算最小编辑距离
    void response(); //响应客户端的请求
};

