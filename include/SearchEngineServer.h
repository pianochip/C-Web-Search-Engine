#pragma once
#include "/home/pianochip/func.h"
#include "nlohmann/json.hpp"
#include <workflow/WFFacilities.h>
#include <workflow/WFTaskFactory.h>
#include <wfrest/HttpServer.h>
#include <sw/redis++/redis++.h>
#include "KeyRecommander.h"
using namespace wfrest;
using namespace std;

class SearchEngineServer
{
public:
    SearchEngineServer(unsigned short port);
    ~SearchEngineServer() {}
    void start();
    void stop();

private:
    wfrest::HttpServer _server; // 使用 wfrest 服务器
    unsigned short _port;
    WFFacilities::WaitGroup _waitGroup;
};

