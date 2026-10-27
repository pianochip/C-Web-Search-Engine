#include "/home/pianochip/func.h"
#include "../include/nlohmann/json.hpp"
#include "../include/KeyRecommander.h"
#include <workflow/WFHttpServer.h>
#include <workflow/HttpUtil.h>
#include <string>
using namespace std;

void process(WFHttpTask* serverTask) {
    //1.获取客户端发来的请求以及服务器要发送的响应
    protocol::HttpRequest* req = serverTask->get_req();
    protocol::HttpResponse* resp = serverTask->get_resp();

    //2.提取http请求的body文本
    size_t len;
    const void *body;
    req->get_parsed_body(&body, &len);

    //3.将JSON文本转为JSON对象
    nlohmann::json jsonobj;
    if(nlohmann::json::accept(static_cast<const char*>(body))){
        jsonobj = nlohmann::json::parse(static_cast<const char*>(body));
    }
    else{
        resp->add_header_pair("Content-Type", "application/json; charset=utf-8");
        resp->append_output_body("{\"status\": \"error\", \"message\": \"无效的请求格式\"}");
        return;
    }

    //4.提取出搜索词
    string queryWord;
    if(jsonobj.contains("query")){
        queryWord = jsonobj["query"];
    }
    else{
        resp->add_header_pair("Content-Type", "application/json; charset=utf-8");
        resp->append_output_body("{\"status\": \"error\", \"message\": \"无效的请求格式\"}");
        return;
    }

    cout << "收到查询词" << queryWord << "\n";

    //5.用KeyRecommander对搜索词进行操作并记录结果
    KeyRecommander recommander(queryWord);
    json respJson = recommander.execute();

    //6.将结果转为HTTP响应报文发回
    resp->add_header_pair("Content-Type", "application/json; charset=utf-8");
    resp->append_output_body(respJson.dump());
}
