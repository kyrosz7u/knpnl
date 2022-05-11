//
// Created by 樱吹雪 on 2022/4/13.
//

#ifndef KHPNL_HTTPSERVER_H
#define KHPNL_HTTPSERVER_H

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "TcpServer.h"
#include <string>
#include <map>

using namespace std;

namespace net {
namespace http {
class HttpServer {
    typedef std::function<void(const HttpRequest *req, HttpResponse &resp)> HttpServeFunc;
public:
    //HttpServer默认起4个工作线程
    HttpServer(int port, int work_nums=1);
    void ListenAndServe(HttpServeFunc f);
private:
    EventLoop mLoop;
    TcpServer mServer;
    const int mPort;

    HttpServeFunc HttpCallback;

    void connectedCb(const TcpConnPtr &conn);

    void readCb(const TcpConnPtr &conn, FixedBuffer *readBuffer, FixedBuffer *writeBuffer);

    void WriteCb(const TcpConnPtr &conn);
};

}//namespace http
}//namespace net

#endif //KHPNL_HTTPSERVER_H
