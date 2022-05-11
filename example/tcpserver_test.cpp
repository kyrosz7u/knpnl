//
// Created by 樱吹雪 on 2022/5/10.
//
#include "net/TcpServer.h"

using namespace net;

void cb(const TcpConnPtr &c, FixedBuffer *buf, FixedBuffer *wbuf);

int main()
{
    EventLoop loop;
    TcpServer s(1234,4,&loop);
    s.SetOnMessageCallback(cb);
    s.Start();
    loop.Loop();
}


void cb(const TcpConnPtr& c, FixedBuffer *rbuf, FixedBuffer *wbuf){
    wbuf->append(rbuf->readPtr(), rbuf->used());
    rbuf->readPtrMove(rbuf->used());
    c->Send();
}