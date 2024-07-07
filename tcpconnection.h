#pragma once
#include"inetaddress.h"
#include"callback.h"
#include"buffer.h"
#include"timestamp.h"
#include<memory>
#include<string>
#include<atomic>
#include"noncopyable.h"
class Channel;
class EventLoop;
class Socket;
/*
 *tcpserver =>acceptor=>有一个新用户连接，通过accept函数拿到connfd
 *=>tcpconnection设置回调，=》channel=>poller=>channel的回调操作
 *
 * */
class TcpConnection:noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
	TcpConnection(EventLoop *Loop,
			const std::string &name,
			int sockfd,
			const InetAddress& LocaLAddr,
			const InetAddress& peerAddr);
	~TcpConnection();
	EventLoop* getLoop() const {return loop_;}
	const std::string &name()const {return name_;}
	const InetAddress& localAddress() const {return localAddr_;}
	const InetAddress& peerAddress() const {return peerAddr_;}

	bool connected() const {return state_==kConnected; }
	//发送数据
	void send(const std::string &buf);
	//关闭连接
	void shutdown();

	void setConnectionCallback(const ConnectionCallback& cb){connectionCallback_=cb;}
	void setMessageCallback(const MessageCallback&cb){messageCallback_=cb;}
	void setWriteCompleteCallback(const WriteCompleteCallback& cb){writeCompleteCallback_=cb;}
	void setHighWaterMarkCallback(const HighWaterMarkCallback& cb,size_t highWaterMark)
	{highWaterMarkCallback_=cb; highWaterMark_=highWaterMark;}
	void setCloseCallback(const CloseCallback& cb)
	{ closeCallback_ = cb; }
	void connectEstablished();//连接建立
	void connectDestroyed();//连接销毁

    
private:
	enum StateE{kDisconnected,kConnecting,kConnected,kDisconnecting};
    void setState(StateE state){state_=state;}
	void handleRead(Timestamp receiveTime);
	void handleWrite();
	void handleClose();
	void handleError();

    
	void sendInLoop(const void* message,size_t Len);
    
    
	void shutdownInLoop();
	EventLoop * loop_;//这里绝对不是baseloop,因为TcpConnection都是在subLoop里面管理的
	const std::string name_;
	std::atomic_int state_;
	bool reading_;

	std::unique_ptr<Socket> socket_;
	std::unique_ptr<Channel> channel_;
	const InetAddress localAddr_;
    const InetAddress peerAddr_;
	ConnectionCallback connectionCallback_;//有新连接时的回调
	MessageCallback messageCallback_;//有读写消息时的回调
	WriteCompleteCallback writeCompleteCallback_;//消息发送完成以后的回调
	HighWaterMarkCallback highWaterMarkCallback_;
	CloseCallback closeCallback_;
    size_t highWaterMark_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
};
