//
// Created by 樱吹雪 on 2022/5/9.
//

#ifndef KHPNL_ACCEPTOR_H
#define KHPNL_ACCEPTOR_H

#include "Socket.h"

#include "event/EventLoop.h"
#include "event/Channel.h"

#include "base/Copyable.h"
#include <memory>

using namespace std;
using namespace base;
using namespace event;

namespace net {
typedef std::function<void(int, sockaddr_in)> newConnCallbackFunc;

/* 功能和需求
 * 1、创建接受新连接的Channel
 * 2、提供连接建立后的回调接口
 * 3、负责ListenFd的创建和关闭*/
class Acceptor : public Noncopyable {
public:
    Acceptor(int port, EventLoop* loop);
    ~Acceptor();
    void Listen();
    bool isListening(){ return mListening; }
    Socket GetSocket(){ return mListenSocket; }
    void SetNewConnCallback(newConnCallbackFunc cb){newConnCallback=cb;}

private:
    void handlerRead();
    bool mListening;
    EventLoop *mLoop;
    unique_ptr<Channel> mChannel;
    Socket mListenSocket;
    newConnCallbackFunc newConnCallback;
};
}// namespace net

#endif //KHPNL_ACCEPTOR_H
