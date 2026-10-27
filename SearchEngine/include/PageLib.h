#pragma once
#include "DirScanner.h"
#include <map>
#include <fstream>
#include <regex>

class PageLib //网页库类
{
public:
    //函数成员
    PageLib(DirScanner& dirScanner);
    ~PageLib() {}
    void create(); //创建网页库
    void store(const char* filepath1, const char* filepath2); //存储网页库和位置偏移库

private:
    //数据成员
    DirScanner& _dirScanner; //目录扫描对象的引用
    vector<string> _files; //存放格式化之后的网页的容器。
    map<int, pair<int, int> > _offsetLib; //存放每篇文档在网页库的位置信息
};

