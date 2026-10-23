#include "/home/pianochip/func.h"
#include "../include/SearchEngineServer.h"

using json = nlohmann::json;

SearchEngineServer::SearchEngineServer(unsigned short port)
    :_port(port), _waitGroup(1)
{
    //注册POST路由
    _server.POST("/search", [this](const wfrest::HttpReq* req, wfrest::HttpResp* resp){
        //判断请求头的类型是否为JSON格式
        if(req->content_type() != APPLICATION_JSON){
            resp->set_status(400);
            resp->String("ERROR: Content-Type is not APPLICATION_JSON");
            return;
        }
        //判断请求体是否为空
        const string& body = req->body();
        if(body.empty()){
            resp->set_status(400);
            resp->String("ERROR: Body is empty");
            return;
        }
        //判断请求体是否符合JSON格式
        if(!json::accept(body)){
            resp->set_status(400);
            resp->String("ERROR: Invalid JSON format");
            return;
        }
        //提取出请求体的JSON文本,找到查询词
        json jsonobj = json::parse(body);
        string queryWord;
        if(!jsonobj.contains("query")){
            resp->set_status(400);
            resp->String("lack query word");
            return;
        }
        queryWord = jsonobj["query"];
        cout << "query: " << queryWord << "\n";

        //查缓存
        WFRedisTask* redisTask = WFTaskFactory::create_redis_task(
            "redis://127.0.0.1:6379",
            10,
            [queryWord, resp](WFRedisTask* redisTask){
                //protocol::RedisRequest* reqRedis = redisTask->get_req();
                protocol::RedisResponse* respRedis = redisTask->get_resp();
                protocol::RedisValue result;
                respRedis->get_result(result);

                //检查查询数据库是否成功
                int state = redisTask->get_state();
                int error = redisTask->get_error();
                if (state != WFT_STATE_SUCCESS) {
                    cerr << "Redis 任务失败! State: " << state << " Error: " << error << endl;
                    resp->set_status(500);
                    resp->String("Redis Error");
                    return; // 如果 Redis 挂了，直接报错返回
                }

                //缓存命中
                if(result.is_string()){
                    cout << "缓存命中\n";
                    resp->headers["Content-Type"] = "application/json";
                    resp->String(result.string_value());
                }
                else{
                //缓存未命中
                    cout << "缓存未命中\n";
                    //使用KeyRecommander查询queryword的候选词
                    KeyRecommander command(queryWord);
                    json respjson = command.execute();
                    string jsonStr = respjson.dump();
                    
                    //将算法结果发送给 HTTP 客户端
                    resp->headers["Content-Type"] = "application/json";
                    resp->String(jsonStr);

                    //将候选词写入redis缓存
                    WFRedisTask* setTask = WFTaskFactory::create_redis_task(
                        "redis://127.0.0.1:6379",
                        10,
                        [](WFRedisTask* setTask){
                            if(setTask->get_state() != WFT_STATE_SUCCESS){
                                cerr << "写回 Redis 失败！\n";
                            }else{
                                cout << "成功写入缓存\n";
                            }
                        }
                    );
                    setTask->get_req()->set_request("SET", {queryWord, jsonStr});
                    series_of(redisTask)->push_back(setTask);
                }
            }
        );

        redisTask->get_req()->set_request("GET", {queryWord});
        redisTask->start();
    });

    //注册GET路由
    _server.GET("/", [](const wfrest::HttpReq* req, wfrest::HttpResp* resp){
        resp->String("Welcome to Search Engine API! Please send POST request to /search.");
    });
}

void SearchEngineServer::start() {
    // 1. 调用 wfrest 服务器的底层 start 方法监听指定端口
    if (_server.start(_port) == 0) {
        cout << ">>> 搜索引擎网络服务启动成功！\n";
        cout << ">>> 正在监听端口: " << _port << "，等待客户端连接...\n";
        
        // 2. 挂起（阻塞）主线程。
        // 如果不加这句，程序瞬间就会跑完退出，服务器也就关了。
        _waitGroup.wait(); 
    } else {
        cerr << ">>> 严重错误：服务启动失败！端口 " << _port << " 可能被占用。" << endl;
    }
    return;
}

void SearchEngineServer::stop() {
    //停止 wfrest 服务器接收新请求
    _server.stop();
    
    _waitGroup.done();
}
