//
// Created by 樱吹雪 on 2022/5/9.
//

#include "TcpConnection.h"

using namespace event;
using namespace net;


void DefaultOnMessageCallback(const TcpConnPtr &conn, FixedBuffer *rbuf, FixedBuffer *wbuf)
{
    rbuf->readPtrMove(rbuf->used());
}

void DefaultWriteCompleteCallback(const TcpConnPtr &conn)
{}


TcpConnection::TcpConnection(int connfd, EventLoop *loop)
    :mLoop(loop),
    mSocket(connfd),
    mChannel(new Channel(connfd,loop))
{
    /* 给Channel注册回调函数时不能用shared_from_this()
     * 否则会导致循环引用，让对象无法自动析构    */
    mChannel->SetReadCallback(std::bind(&TcpConnection::handleRead, this));
    mChannel->SetWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    mChannel->SetCloseCallback(std::bind(&TcpConnection::handleClose, this));

    memset(&m_iv, 0, sizeof m_iv);  // 将iovec清零
}

TcpConnection::~TcpConnection()
{
    close(mSocket.Fd());
}

// 可以被其他线程调用
void TcpConnection::Send()
{
    if(mLoop->IsInLoopThread()){
        sendInLoop();
    }else {
        mLoop->RunInLoop(std::bind(&TcpConnection::sendInLoop, this));
    }
}

// 主动关闭
void TcpConnection::Close()
{
    mLoop->RunInLoop(std::bind(&TcpConnection::closeInLoop, this));
}

/* 首先尝试直接发送，如果没发完等待mChannel可读之后，
 * Poller唤醒线程后调用handleWrite()，
 * 这样做的好处是可以减少Poller的触发   */
void TcpConnection::sendInLoop()
{
    int ret = sendMessage();
    if(ret==-1){
        handleClose();
        return ;
    }else if(ret>0){    //还没写完
        mChannel->EnableWrite();
    }else if(ret==0){   // 发送完毕
        if(writeCompleteCallback){
            mLoop->QueueInLoop(
        std::bind(writeCompleteCallback,shared_from_this()));
        }
    }
}

/* 先关闭写端，等待收到对端发送的FIN，
 * 然后recvMessage返回0，由handleRead执行关闭操作*/
void TcpConnection::closeInLoop()
{
    mSocket.shutdownWrite();
}

/* 该函数是在Eventloop::Loop()中调用，
 * 为了保证同一个IO线程其他的连接及时从系统缓冲区中读走数据
 * 要采用QueueInLoop的方式执行回调函数*/
void TcpConnection::handleRead()
{
    long int ret = recvMessage();
    if(ret<=0){
        handleClose();
        return ;
    }
    LOG_TRACE<<"recvMessage:"<<mReadBuffer.readPtr();
    if(onMessageCallback){
        onMessageCallback(shared_from_this(),&mReadBuffer, &mWriteBuffer);
    }else{
        DefaultOnMessageCallback(shared_from_this(), &mReadBuffer, &mWriteBuffer);
    }
}

void TcpConnection::handleWrite()
{
    long int ret = sendMessage();
    if(ret==-1){
        handleClose();
        return ;
    }else if(ret>0){    //还没写完
        return ;
    }else if(ret==0){   // 发送完毕
        if(writeCompleteCallback){
            mLoop->QueueInLoop(
            std::bind(writeCompleteCallback,shared_from_this()));
        }else{
            DefaultWriteCompleteCallback(shared_from_this());
        }
        mChannel->DisableWrite();
    }
}

/* 处理连接关闭*/
void TcpConnection::handleClose()
{
    LOG_TRACE << "handleClose: ";
    if(mState==CONNECTING||mState==CONNECTED){
        mState = CLOSING;
        mLoop->AssertInLoop();  // 操作Channel之前要保证在对应的线程
        mChannel->DisableAll(); // 这里停止发送和接收
        TcpConnPtr guard(shared_from_this());
        closeCallback(guard);
    }

}

/* 处理连接创建后的初始化工作
 * 因为TcpConnection对象可能在其他的线程中创建
 * 故初始化不能在构造函数中完成   */
void TcpConnection::Established()
{
    LOG_TRACE << "Established: ";
    mLoop->AddChannel(mChannel.get());
    mChannel->EnableRead();
    if(onConnectCallback){
        TcpConnPtr guard(shared_from_this());
        onConnectCallback(guard);
    }
    mState = CONNECTED;
}

/* 将channel从Poller中去除，避免造成内存泄露*/
void TcpConnection::Destroy()
{
    LOG_TRACE << "Destroy: ";
    assert(mChannel->isIdle());
    mChannel->Remove();
    mState = CLOSED;
}

/*读取收到的数据，并存入mReadBuffer*/
long int TcpConnection::recvMessage() {
    return mReadBuffer.readFd(mSocket.Fd());
}

/* 将mWriteBuffer中的数据发送出去*/
/* 返回 -1：出错  0：发送完毕   >0：待发送数量*/
long int TcpConnection::sendMessage() {
    long int write_bytes;
    long int write_sum=0;

    while(1){
        int count = 2;
        m_iv[0].iov_base=(char*)mWriteBuffer.readPtr();
        m_iv[0].iov_len = mWriteBuffer.used();
        if (m_iv[1].iov_base == nullptr) {
            count=1;
        }
        write_bytes = ::writev(mSocket.Fd(), m_iv, 2);
        LOG_TRACE << write_bytes;
        if(write_bytes<=0){
            break;
        }
        if(write_bytes>mWriteBuffer.used()){
            size_t temp = write_bytes - mWriteBuffer.used();
            m_iv[1].iov_base = (char*)(m_iv[1].iov_base) + temp;
            m_iv[1].iov_len -= temp;
            mWriteBuffer.reset();
        }else{
            mWriteBuffer.readPtrMove(write_bytes);
        }
        write_sum+=write_bytes;
    }

    if(write_bytes==-1){
        if (errno == EAGAIN) {
            return mWriteBuffer.used() + m_iv[1].iov_len;
        }
        LOG_ERROR << "errno"<<sys_errlist[errno];
        LOG_ERROR<<"m_iv[0] addr: "<<m_iv[0].iov_base;
        LOG_ERROR<<"m_iv[0] len: "<<m_iv[0].iov_len;
        LOG_ERROR<<"m_iv[1] addr: "<<m_iv[1].iov_base;
        LOG_ERROR<<"m_iv[1] len: "<<m_iv[1].iov_len;
    }
    m_iv[1].iov_base= nullptr;
    m_iv[1].iov_len= 0;
    return write_bytes;
}

