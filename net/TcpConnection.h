//
// Created by 樱吹雪 on 2022/5/9.
//

#ifndef KHPNL_TCPCONNECTION_H
#define KHPNL_TCPCONNECTION_H

#include "Socket.h"
#include "base/Buffer.h"
#include "base/FileSystem.h"
#include "base/Logger.h"
#include "base/Copyable.h"

#include "event/EventLoop.h"
#include "event/Channel.h"

#include "boost/any.hpp"
#include <sys/uio.h>
#include <memory>
#include <utility>

using namespace base;
using namespace std;
using namespace event;


namespace net {
// 类型前向声明
class TcpConnection;
class Socket;

// 定义指向连接的智能指针
typedef shared_ptr<TcpConnection> TcpConnPtr;
// 定义处理网络事件的回调函数
typedef function<void(const shared_ptr<TcpConnection>&)> ConnCallbackFunc;
typedef function<void(const shared_ptr<TcpConnection>&,
        FixedBuffer*, FixedBuffer*)> MessageCallbackFunc;   // 前面是readBuffer，后面的是writeBuffer


/*功能需求：
 * 1、实现基于用户态缓存的网络连接读写
 * 2、管理Socket对象和其对于的Channel
 * 3、提供网络事件发生时，用来处理的回调函数
 * 4、能够保存上下文
 * 5、安全且优雅的关闭连接
 * TcpConnection可以保证关闭之前接收到网络上所有的数据包，
 * 但不保证业务数据的完整性。即收到对端FIN包后，直接关闭连接，WriteBuffer里面
 * 没发送完的数据不再发送；当本端需要关闭的时候，先关闭写端，接收到对端的FIN后再关闭
 * 整个连接，这样要考虑对方收到FIN后继续发送这种情况，解决办法是：添加一个定时任务，
 * 超过一定时间后强制关闭 */
class TcpConnection: public Noncopyable,
                     public enable_shared_from_this<TcpConnection>  // 必须要public继承
{
public:
    TcpConnection(int connfd,EventLoop *loop);

    ~TcpConnection();

    EventLoop* getLoop(){ return mLoop; }
    int getSocketFd(){ return mSocket.Fd(); }

    void Send();
    void Close();

    boost::any& getContext(){ return mContext; }
    boost::any* getMutableContext(){return &mContext;}
    void setContext(const boost::any &c){ mContext = c; }
    void setFile(FileSystem &F){
        m_iv[1].iov_base= F.getAddr();
        m_iv[1].iov_len = F.getLength();
    }
    void SetAlive(bool on){ mAlive = on; }
    bool GetAlive(){ return mAlive; }

    void SetOnConnectCallback(ConnCallbackFunc cb){ onConnectCallback = std::move(cb); }
    void SetOnMessageCallback(MessageCallbackFunc cb){ onMessageCallback = std::move(cb); }
    void SetWriteCompleteCallback(ConnCallbackFunc cb){ writeCompleteCallback = std::move(cb); }
    void SetCloseCallback(ConnCallbackFunc cb){ closeCallback = std::move(cb); }

    void Established();
    void Destroy();
private:
    EventLoop* mLoop;
    Socket mSocket;
    shared_ptr<Channel> mChannel;
    bool mAlive;
    FixedBuffer mReadBuffer;
    FixedBuffer mWriteBuffer;
    boost::any mContext;
    struct iovec m_iv[2];
private:
    friend class TcpServer;
    ConnCallbackFunc onConnectCallback;
    MessageCallbackFunc onMessageCallback;
    ConnCallbackFunc writeCompleteCallback;
    ConnCallbackFunc closeCallback;

    void handleRead();
    void handleWrite();
    void handleClose();

    long int sendMessage();

    long int recvMessage();

    void sendInLoop();

    void closeInLoop();
};
}// namespace net

#endif //KHPNL_TCPCONNECTION_H
