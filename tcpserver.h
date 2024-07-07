#pragma once

#include "eventloop.h"
#include"acceptor.h"
#include"inetaddress.h"
#include"noncopyable.h"
#include<functional>
#include<string>
#include<memory>
#include"eventloopthreadpool.h"
#include"callback.h"
#include<atomic>
#include<unordered_map>
//#
class TcpServer:noncopyable
{
public:
    using ThreadInitCallback=std::function<void(EventLoop*)>;
    enum Option
    {
        kNoReusePort,
        kReusePort,    
    };
    TcpServer(EventLoop *Loop,
                const InetAddress &ListenAddr,
                const std::string &namearg,
                Option option=kNoReusePort);
    ~TcpServer();
    void setThreadInitCallback(const ThreadInitCallback &cb){threadInitCallback_=cb;}
    void setConnectionCallback(const ConnectionCallback &cb){connectionCallback_=cb;}
    void setMessageCallback(const MessageCallback &cb){messageCallback_=cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback &cb){writeCompleteCallback_=cb;}
    void setThreadNum(int numThreads);
    void start();//start a listen //mainloop 
private:
    void newConnection(int sockfd,const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr & conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);
    using ConnectionMap=std::unordered_map<std::string,TcpConnectionPtr>;
    EventLoop *loop;
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor>  acceptor_;
    std::shared_ptr<EventloopThreadPool> threadPool_;//thread pool 
    ConnectionCallback connectionCallback_;//huidiao new connection
    MessageCallback messageCallback_;//huidiao write connection
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;
    std::atomic_int started_;
    int nextConnId_;
    ConnectionMap connections_;//save all connection

};
