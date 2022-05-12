//
// Created by 樱吹雪 on 2022/5/9.
//

#include "TcpServer.h"

using namespace event;
using namespace net;

using std::placeholders::_1;
using std::placeholders::_2;

TcpServer::TcpServer(int port, int nums, EventLoop *loop)
    :mLoop(loop),
    mThreadPool(new LoopThreadPool(nums)),
    mAcceptor(new Acceptor(port,mLoop))
{
    mAcceptor->SetNewConnCallback(
            std::bind(&TcpServer::newConnection,this,_1,_2));
}

TcpServer::~TcpServer()
{
    for (auto Entry:connMap) {
        TcpConnPtr conn = Entry.second;
        // 释放掉map中智能指针的引用
        Entry.second.reset();
        EventLoop *ioLoop = conn->getLoop();
        // 注意这里用到bind()可以让conn的生命期延长到执行完Established
        ioLoop->RunInLoop(std::bind(&TcpConnection::Established,conn));
    }
}

void TcpServer::Start()
{
    assert(!mAcceptor->isListening());
    LOG_INFO<<"TcpServer Listening...";
    mLoop->RunInLoop(std::bind(&Acceptor::Listen, mAcceptor.get()));
}

void TcpServer::Close(const TcpConnPtr &conn)
{
    int connfd = conn->getSocketFd();
    auto it=connMap.find(connfd);
//    assert(it!=connMap.end());
    if(it!=connMap.end())
    {
        EventLoop *ioLoop = conn->getLoop();
        LOG_TRACE << "close connection: " << connfd;
        ioLoop->QueueInLoop(std::bind(&TcpConnection::handleClose, conn));
        LOG_TRACE << "conn use count: " << conn.use_count();
    }
}

void TcpServer::newConnection(int connfd, sockaddr_in addr)
{
    mLoop->AssertInLoop();  // 保证connmMap不会出现线程安全问题
    EventLoop *ioLoop=mThreadPool->getNextLoop();
    TcpConnPtr conn(new TcpConnection(connfd, ioLoop));
    LOG_TRACE << "new connection: " << connfd;
    connMap[connfd] = conn;

    conn->SetOnConnectCallback(onConnectCallback);
    conn->SetOnMessageCallback(onMessageCallback);
    conn->SetWriteCompleteCallback(writeCompleteCallback);
    conn->SetCloseCallback(std::bind(&TcpServer::removeConnection,this,_1));

    conn->mState = CONNECTING;
    ioLoop->RunInLoop(std::bind(&TcpConnection::Established,conn));
    LOG_TRACE << "conn use count: " << conn.use_count();
}

void TcpServer::removeConnection(const TcpConnPtr &conn)
{
    mLoop->RunInLoop(std::bind(&TcpServer::removeInLoop, this, conn));
    LOG_TRACE << "conn use count: " << conn.use_count();
}
void TcpServer::removeInLoop(const TcpConnPtr &conn)
{
    mLoop->AssertInLoop();  // 保证connMap不会出现线程安全问题
    EventLoop *ioLoop = conn->getLoop();
    int connfd = conn->getSocketFd();
    size_t sz = connMap.erase(connfd);
    assert(sz==1);

    LOG_TRACE << "remove connection: " << connfd;
    /* 这里用queueInLoop是为了保证当mLoop和ioLoop相同的时候，
     * Tcp连接的剩余IO事件能被处理，相当于慢关闭*/
    ioLoop->QueueInLoop(std::bind(&TcpConnection::Destroy, conn));
    LOG_TRACE << "conn use count: " << conn.use_count();
}

void AddrToString(const sockaddr_in &addr)
{

}
