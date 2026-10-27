#include "../include/DictProducer.h"
#include "../include/SplitToolJieba.h"
#include "../include/Dictionary.h"
#include "../include/KeyRecommander.h"
#include "../include/SearchEngineServer.h"
#include "../include/DirScanner.h"
#include "../include/PageLib.h"
#include "../include/PageLibPreprocessor.h"
#include "../include/WebPage.h"
#include "../include/WebPageQuery.h"

int main(int argc, char** argv){
    remove("../data/dict.dat");
    remove("../data/dictIndex.dat");

    DictProducer enDict("../data/yuliao/english.txt");//读取英文语料
    enDict.buildEnDict();//创建英文词典
    enDict.storeDict("../data/dict.dat");//将词典存到本地
    enDict.createIndex(); //创建词典索引
    enDict.storeIndex("../data/dictIndex.dat");//将词典索引存到本地
    
    //计算中文词典索引偏移
    int offset = 0;
    ifstream ifs("../data/dict.dat");
    string dummy;
    while(getline(ifs, dummy)) {
        offset++;
    }
    ifs.close();

    SplitToolJieba* tool = new SplitToolJieba;
    DictProducer zhDict("../data/yuliao/art", tool);//读取中文语料
    zhDict.dictoffset = offset;
    zhDict.buildCnDict();//创建中文词典
    zhDict.storeDict("../data/dict.dat");//将词典存到本地
    zhDict.createIndex(); //创建词典索引
    zhDict.storeIndex("../data/dictIndex.dat");//将词典索引存到本地

    Dictionary* dictObj = Dictionary::createInstance();
    dictObj->init("../data/dict.dat", "../data/dictIndex.dat");
    // auto& myDict = dictObj->getDict();
    // auto& myIndexTable = dictObj->getIndexTable();

    //构建网页库和偏移库
    DirScanner scanner;
    PageLib pageLib(scanner);
    pageLib.create();
    pageLib.store("../data/newripepage.dat", "../data/newoffset.dat");
    cout << ">>> 网页库 (newripepage.dat) 和偏移库 (newoffset.dat) 构建完成！" << "\n";

    //网页预处理
    cout << ">>> 开始网页预处理(查重与倒排索引构建)..." << "\n";
    PageLibPreprocessor preprocessor;
    preprocessor.doProcess();

    SearchEngineServer server(8080);
    server.start();

    return 0;
}

