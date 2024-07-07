#include"tcpserver.h"
#include"Logger.h"
#include<functional>
#include"tcpconnection.h"
#include<strings.h>
#include<functional>
EventLoop* CheckLoopNotNull(EventLoop * Loop)
{
    if(Loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainLoop is null! \n",__FILE__,__FUNCTION__,__LINE__);    
    }
    return Loop;
}
TcpServer::TcpServer(EventLoop *Loop,
                const InetAddress &ListenAddr,
                const std::string &namearg,
                Option option)
    :loop(CheckLoopNotNull(Loop))
    ,ipPort_(ListenAddr.toIpPort())
    ,name_(namearg)
    ,acceptor_(new Acceptor(loop,ListenAddr,option==kReusePort))
    ,threadPool_(new EventloopThreadPool (loop,name_))
    ,connectionCallback_()
    ,nextConnId_(1)
{
//while new user connect,start tcpserver::onconnection
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,
        std::placeholders::_1,std::placeholders::_2));
}

void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);

}
void TcpServer::start()
{
    if(started_++==0)
    {
        threadPool_->start(threadInitCallback_);
        loop->runInLoop(std::bind(&Acceptor::listen,acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd,const InetAddress &peerAddr)
{
	// 轮询算法，选择一个subLoop，来管理channel
EventLoop *ioLoop = threadPool_->getNextLoop(); 
char buf[64] = {0};
snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
++nextConnId_;
std::string connName = name_ + buf;

LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s \n",
    name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

// 通过sockfd获取其绑定的本机的ip地址和端口信息
sockaddr_in local;
::bzero(&local, sizeof local);
socklen_t addrlen = sizeof local;
if (::getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0)
{
    LOG_ERROR("sockets::getLocalAddr");
}
InetAddress localAddr(local);

// 根据连接成功的sockfd，创建TcpConnection连接对象
TcpConnectionPtr conn(new TcpConnection(
                        ioLoop,
                        connName,//连接的名字
                        sockfd,   // Socket Channel
                        localAddr,
                        peerAddr));
connections_[connName] = conn;//将连接conn放入到connections是一个unordered_map<string ,conn>的哈希表
// 下面的回调都是用户设置给TcpServer=>TcpConnection=>Channel=>Poller=>notify channel调用回调
conn->setConnectionCallback(connectionCallback_);
conn->setMessageCallback(messageCallback_);
conn->setWriteCompleteCallback(writeCompleteCallback_);

// 设置了如何关闭连接的回调   conn->shutDown()
conn->setCloseCallback(
    std::bind(&TcpServer::removeConnection, this, std::placeholders::_1)
);

// 直接调用TcpConnection::connectEstablished
ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));//调用ioloop的事件回调    
}
void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop->runInLoop(//传入删除连接回调
        std::bind(&TcpServer::removeConnectionInLoop, this, conn)
    );
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s\n", 
        name_.c_str(), conn->name().c_str());

    connections_.erase(conn->name());//在连接表中删除这个连接
    EventLoop *ioLoop = conn->getLoop(); //获取下一个连接
    ioLoop->queueInLoop(//执行删除连接的回调
        std::bind(&TcpConnection::connectDestroyed, conn)
    );
}




