#include "/home/pianochip/func.h"
#include "../include/PageLib.h"


//构造函数
PageLib::PageLib(DirScanner& dirScanner)
: _dirScanner(dirScanner) {}

//创建网页库
void PageLib::create() {
    //1.取得所有语料的绝对路径
        //调用DirScanner方法取得语料的路径
    _dirScanner.traverse("../data/xml");
    vector<string>& pathFile = _dirScanner.getFiles();

    //2.打开文件，按行读取文件
        //循环打开路径文件里所有路径的文件
    int docid = 1;
    for(size_t i=0; i<pathFile.size(); i++){
        ifstream xml_corpus(pathFile[i]); //读取此路径的文件
        if(!xml_corpus.is_open()){ //检查文件是否打开
            cerr << pathFile[i] << "打开失败\n";
            continue;
        }

        //按行读取文件
        string firstline; //暂存第一行的内容,若没有标题则取第一行为标题
        string line;
        string fmtTxt; //拼接整个文档的内容
        string url;
        string title;
        string content;
        
        //3.将文件内的数据格式化。转为可识别的网页格式
        regex html_regex("<[^>]*>");
        while(getline(xml_corpus, line)){
            //找到了文章开头
            if(line.find("<item>") != string::npos) {
                url.clear();
                title.clear();
                content.clear();
            }
            //提取title
            else if(line.find("<title>") != string::npos) {
                size_t start = line.find("CDATA[") + strlen("CDATA[");
                size_t end = line.find("]]></title>");
                title = line.substr(start, end - start);
            }
            //提取url
            else if(line.find("<link>") != string::npos) {
                size_t start = line.find("<link>") + strlen("<link>");
                size_t end = line.find("</link>");
                url = line.substr(start, end - start);
            }
            //提取正文内容并清洗 HTML
            else if(line.find("<description>") != string::npos) {
                size_t start = line.find("CDATA[") + strlen("CDATA[");
                size_t end = line.find("]]></description>");
                if (start != string::npos && end != string::npos) {
                    string raw_content = line.substr(start, end - start);
                    // 使用正则将 "<p...>" 和 "</p>" 全部替换为空字符串
                    content = regex_replace(raw_content, html_regex, ""); 
                }
            }
            //文章结尾,开始拼接
            else if(line.find("</item>") != string::npos){
                string final_content = title + " " + content;
                string docStr = "<doc><docid>" + to_string(docid) + "</docid>"
                              + "<url>" + url + "</url>"
                              + "<title>" + title + "</title>"
                              + "<content>" + final_content + "</content></doc>";
                
                //4.将格式化后的数据存入_files
                _files.push_back(docStr);
                ++docid;
            }
        }
    }  
}

//存储网页库和位置偏移库
void PageLib::store(const char* filepath1, const char* filepath2) {
    ofstream ofs1(filepath1, std::ios::out | std::ios::trunc); //打开网页库
    ofstream ofs2(filepath2, std::ios::out | std::ios::trunc); //打开网页偏移库
    if (!ofs1.is_open() || !ofs2.is_open()) {
        std::cerr << "文件打开失败！" << std::endl;
        return;
    }

    int docid = 1;
    //1.循环遍历_files
    for(size_t i=0; i<_files.size(); i++){
        int offset = ofs1.tellp(); 
        ofs1 << _files[i] << "\n";
        int length = _files[i].size() + 1; 

        //将当前的网页偏移和长度存入偏移库
        _offsetLib[docid] = {offset, length};
        ofs2 << docid << " " << offset << " " << length << "\n";
        
        docid++;
    }
    ofs1.close();
    ofs2.close();
}

