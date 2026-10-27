#pragma once
#include "SplitTool.h"
#include "../3rdpart/cppjieba/include/cppjieba/Jieba.hpp"

class SplitToolJieba : public SplitTool{
public:
    SplitToolJieba();
    ~SplitToolJieba();
    vector<string> cut(const string& sentence) override;
private:
    cppjieba::Jieba _jieba;
};