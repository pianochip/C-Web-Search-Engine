#pragma once
#include <string>
#include <vector>
#include <filesystem>
using namespace std;

class DirScanner //目录扫描类
{
public:
    DirScanner() {};
    ~DirScanner() {}
    void traverse(const string &dirname); //获取某一目录下所有文件的路径
    vector<string>& getFiles(); //取得_files
private:
    vector<string> _files; //存放每个语料文件的绝对路径
    
};

