//
// Created by 樱吹雪 on 2022/5/9.
//

#include "Socket.h"
#include <sys/socket.h>
#include "arpa/inet.h"
#include <fcntl.h>
#include <assert.h>

namespace net{
int CreateSocketFd()
{
    int socketfd=socket(PF_INET, SOCK_STREAM, 0);

    return socketfd;
}

void Socket::Listen()
{
    //LISTEN_BACKLOG设置内核监听队列的最大长度。
    //当监听长度大于LISTEN_BACKLOG时，服务器将不再受理新的客户连接，
    //客户端将收到ECONNREFUSED错误信息。
    int ret = ::listen(mFd, SOMAXCONN); // SOMAXCONN=4096
    assert(ret==0);
}

void Socket::Bind(const StringPiece &ip, int port)
{
    //表示socket网络地址的结构体，需要设置好协议、地址和端口
    sockaddr_in addr;
    bzero(&addr,sizeof addr);
    addr.sin_family=AF_INET;

    inet_aton(ip.data(), &addr.sin_addr);
    addr.sin_port=htons(port);

    int ret=bind(mFd, (sockaddr*)&addr, sizeof addr);
    assert(ret==0);
    mBinded = true;
}

int Socket::Accept(sockaddr_in &client_addr){
    socklen_t addr_len;
    addr_len = sizeof client_addr;
    // 将connfd设置为非阻塞模式
    int connfd=::accept4(mFd, (sockaddr*)&client_addr, &addr_len,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);
    return connfd;
}

void Socket::shutdownWrite()
{
    shutdown(mFd, SHUT_RD);
}

void Socket::setReusePort(bool on)
{
    int enable=1;
    ::setsockopt(mFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof (int));
}

void Socket::setReuseAddr(bool on)
{
    int optval = 1;
    ::setsockopt(mFd, SOL_SOCKET, SO_REUSEADDR,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

int Socket::setNonblock()
{
    int old_option=fcntl(mFd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;

    fcntl(mFd, F_SETFL, new_option);
    return old_option;
}

}


