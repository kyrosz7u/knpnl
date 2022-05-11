//
// Created by 樱吹雪 on 2022/5/9.
//

#ifndef KHPNL_TCPSERVER_H
#define KHPNL_TCPSERVER_H

#include "TcpConnection.h"
#include "Acceptor.h"
#include "event/LoopThreadPool.h"
#include <map>
#include <functional>
#include <utility>

using namespace std;
using namespace event;
using namespace net;

/* 功能需求：
 * 1、用恰当的数据结构管理连接
 * 2、为每个连接分配IoLoop
 * 3、正确关闭连接 */
namespace net {
class TcpServer {
public:
    TcpServer(int port, int nums, EventLoop *loop);

    ~TcpServer();

    void Start();

    void Close(const TcpConnPtr &connPtr);   // 还需要考虑下对象生命期

    void SetOnConnectCallback(ConnCallbackFunc cb){ onConnectCallback = std::move(cb); }
    void SetOnMessageCallback(MessageCallbackFunc cb){ onMessageCallback = std::move(cb); }
    void SetWriteCompleteCallback(ConnCallbackFunc cb){ writeCompleteCallback = std::move(cb); }

    void newConnection(int connfd, sockaddr_in addr);
    void removeConnection(const TcpConnPtr &conn);
private:
    EventLoop *mLoop;
    unique_ptr<LoopThreadPool> mThreadPool;
    unique_ptr<Acceptor> mAcceptor;
    map<int, TcpConnPtr> connMap;

    ConnCallbackFunc onConnectCallback;
    MessageCallbackFunc onMessageCallback;
    ConnCallbackFunc writeCompleteCallback;

    void removeInLoop(const TcpConnPtr &conn);
};
}// namespace net

#endif //KHPNL_TCPSERVER_H
