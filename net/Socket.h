//
// Created by 樱吹雪 on 2022/5/9.
//

#ifndef KHPNL_SOCKET_H
#define KHPNL_SOCKET_H

/* 功能需求
 * 1、封装好Socket API
 * 2、负责管理好Socket fd的生命期*/
#include "base/StringPiece.h"
#include "base/Copyable.h"
#include <netinet/in.h>

using namespace base;

namespace net {
int CreateSocketFd();
class Socket;
class Socket: public Copyable{
public:
    // Socket类不创建文件描述符，只管理
    Socket(int fd)
    {
        mFd = fd;
        mBinded= false;
    }
    inline int Fd(){ return mFd; }

    void Bind(const StringPiece &ip, int port);

    bool isBinded(){ return mBinded; }

    void Listen();

    int Accept(sockaddr_in &client_addr);

    void shutdownWrite();

    void setReusePort(bool on);

    void setReuseAddr(bool on);

    int setNonblock();

private:
    int mFd;
    bool mBinded;
};
}// namespace net

#endif //KHPNL_SOCKET_H
