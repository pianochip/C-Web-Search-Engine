#pragma once
#include <vector>
#include <string>
#include <map>
#include <set>
using namespace std;

class SplitTool
{
public:
    SplitTool() {}
    virtual ~SplitTool() {}
    virtual vector<string> cut(const string& sentence)=0;
private:

};

