//
// Created by 樱吹雪 on 2022/5/9.
//

#include "Acceptor.h"
#include "event/Channel.h"

using namespace event;
using namespace net;

Acceptor::Acceptor(int port, EventLoop* loop)
    : mListenSocket(CreateSocketFd()),
    mLoop(loop),
    mListening(false)
{
    mListenSocket.setNonblock();
    mListenSocket.setReuseAddr(true);
    mListenSocket.setReusePort(true);

    mListenSocket.Bind("0.0.0.0", port);

    mChannel.reset(new Channel(mListenSocket.Fd(),loop));
    mLoop->AddChannel(mChannel.get());
    mChannel->SetReadCallback(std::bind(&Acceptor::handlerRead, this));
}

Acceptor::~Acceptor()
{
    mChannel->DisableAll();
    mChannel->Remove();
    close(mListenSocket.Fd());
}

void Acceptor::Listen()
{
    assert(mListenSocket.isBinded());
    mListening = true;
    mListenSocket.Listen();
    mChannel->EnableRead();
}

void Acceptor::handlerRead()
{
    sockaddr_in addr;
    // epoll边沿触发需要这样处理
    while(1)
    {
        int connfd=mListenSocket.Accept(addr);
        if(connfd>=0){
            if(newConnCallback){
                newConnCallback(connfd,addr);
            }else{
                close(connfd);
            }
        }else{
            if (errno != EAGAIN) {
                LOG_ERROR << "errno"<<sys_errlist[errno];
            }
            break;
        }
    }
}


