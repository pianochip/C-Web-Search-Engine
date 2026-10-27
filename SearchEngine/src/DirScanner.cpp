#include "/home/pianochip/func.h"
#include "../include/DirScanner.h"

void DirScanner::traverse(const string &dir){
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
        for (const auto& entry : fs::recursive_directory_iterator(dir)) {
            // 过滤掉可能存在的子文件夹，只提取普通文本文件
            if (entry.is_regular_file()) {
                // entry.path().string() 能将路径对象转换回标准的 std::string
                _files.push_back(entry.path().string());
            }
        }
    }
}

vector<string>& DirScanner::getFiles() { return _files; }