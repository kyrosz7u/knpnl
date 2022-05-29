//
// Created by 樱吹雪 on 2022/4/13.
//

#include "HttpServer.h"
#include "HttpContext.h"
#include "net/TcpConnection.h"
#include "base/Logger.h"
#include <functional>

using namespace net;
using namespace net::http;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

HttpServer::HttpServer(int port, int work_nums)
        : mPort(port),
          mServer(port,work_nums,&mLoop)
{
    mServer.SetOnConnectCallback(
            std::bind(&HttpServer::connectedCb, this, _1));
    mServer.SetOnMessageCallback(
            std::bind(&HttpServer::onMessageCallback, this, _1, _2, _3));
    mServer.SetWriteCompleteCallback(
            std::bind(&HttpServer::WriteCompleteCallback, this, _1));
}

void HttpServer::connectedCb(const TcpConnPtr &conn){
    conn->setContext(HttpContext());
    LOG_TRACE << "TcpServer::newConnection "<< conn->getSocketFd();
};

void HttpServer::onMessageCallback(const TcpConnPtr &conn,
                                   FixedBuffer *readBuffer,
                                   FixedBuffer *writeBuffer)
{
    /* 为什么是any_cast<HttpContext> */
    HttpContext *ctx=boost::any_cast<HttpContext>(conn->getMutableContext());
    HttpRequest *req = ctx->getRequest();
    HttpResponse &resp = ctx->getResponse();

    HTTP_CODE ret= req->processRead(readBuffer);
    if(ret==NO_REQUEST){
        LOG_TRACE << "NO_REQUEST " << conn->getSocketFd();
        return ;
    }
    else if(ret==BAD_REQUEST){
        LOG_TRACE << "BAD_REQUEST " << conn->getSocketFd();
        resp.setStatusCode(HttpResponse::CLIENT_ERROR_BAD_REQUEST);
        conn->SetAlive(false);
        resp.appendToBuffer(writeBuffer);
    }else if(ret==GET_REQUEST) {
        LOG_TRACE << "GET_REQUEST" << conn->getSocketFd();
        resp.setStatusCode(HttpResponse::SUCCESS_OK);
        // http回调函数
        HttpCallback(req, resp);
        resp.appendToBuffer(writeBuffer);
        // 设置要传输的文件
        if(resp.hasFile()){
            conn->setFile(resp.getFile());
        }
        conn->SetAlive(resp.isAlive & req->isAlive);
        req->Init();
    }
    conn->Send();
}
//
void HttpServer::WriteCompleteCallback(const TcpConnPtr &conn){
    if (!conn->GetAlive()) {
        mServer.Close(conn);
    }
}

void HttpServer::ListenAndServe(HttpServeFunc f){
    HttpCallback = f;
    mServer.Start();
    mLoop.Loop();
}













