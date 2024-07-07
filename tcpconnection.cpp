#include"tcpconnection.h"
#include"Logger.h"
#include"socket.h"
#include"channel.h"
#include"eventloop.h"
#include<netinet/tcp.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<strings.h>
static EventLoop* CheckLoopNotNull(EventLoop * Loop)
{
    if(Loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainLoop is null! \n",__FILE__,__FUNCTION__,__LINE__);    
    }
    return Loop;
}

TcpConnection::TcpConnection(EventLoop *Loop,
			const std::string &name,
			int sockfd,
			const InetAddress& LocaLAddr,
			const InetAddress& peerAddr)
    :loop_(CheckLoopNotNull(Loop))
    ,name_(name)
    ,state_(kConnecting)
    ,reading_(true)
    ,socket_(new Socket(sockfd))
    ,channel_(new Channel(Loop,sockfd))
    ,localAddr_(localAddr_)
    ,peerAddr_(peerAddr_)
    ,highWaterMark_(64*1024*1024)// 64M
{
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead,this,std::placeholders::_1)
    );
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite,this)   
    );
    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose,this)
    );
    channel_->setErrorCallback(
          std::bind(&TcpConnection::handleError,this)
    );
    LOG_INFO("TcpConnection::ctor[%s] at fd=%d\n",name_.c_str(),sockfd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d \n",name_.c_str(),channel_->fd(),(int)state_);
}

void TcpConnection::send(const std::string &buf)
{
	if(state_==kConnected)
	{
		if(loop_->isInLoopThread())
		{
			sendInLoop(buf.c_str(),buf.size());
			
		}
		else
		{
			loop_->runInLoop(std::bind(
						&TcpConnection::sendInLoop,
						this,
						buf.c_str(),
						buf.size()
						));
		}
	}
}

void TcpConnection::sendInLoop(const void*data,size_t len)
{
	ssize_t nwrote=0;
	size_t remaining=len;
	bool faultError=false;
	//之前调用过该connection的shutdown，不能再进行发送了
	if(state_==kDisconnecting)
	{
		LOG_ERROR("disconnected,give up writing!");
		return;
	}
	//表示channel——第一次开始写数据，而且缓冲区没有待发送数据
	if(!channel_->isWriting() && outputBuffer_.readableBytes()==0)
	{
		nwrote=::write(channel_->fd(),data,len);
		if(nwrote>=0)
		{
			remaining=len-nwrote;
			if(remaining ==0 && writeCompleteCallback_)
			{	//既然再这里数据全部发送完成们就不用给channel设置epoll事件了
				loop_->queueInLoop(
					std::bind(writeCompleteCallback_,shared_from_this())
						);
			}
		}
		else
		{
			nwrote=0;
			if(errno!=EWOULDBLOCK)
			{
				LOG_ERROR("Tcpconnection ::sendInLoop");
				if(errno==EPIPE ||errno==ECONNRESET)
				{
					faultError=true;
				}
			}
		}
	}
	if(!faultError && remaining>0)//当前这一ICwrite,并没有把数据全部发送出去，剩余的数据需要保存再缓冲区当中，然后给channel
				      //注册epollout事件，poller发现tcp的发送缓冲区有空间，回通知相应的sockchannel,调用writecallback回调方法
				      //也就是调用tcpconnection::handlewrite方法，把发送缓冲区中的数据全部发送完成
	{
		size_t oldLen = outputBuffer_.readableBytes();
		if(oldLen+remaining>=highWaterMark_ && oldLen<highWaterMark_ && highWaterMarkCallback_)
		{
			loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(),oldLen+remaining));
		}
		outputBuffer_.append((char*)data+nwrote,remaining);
		if(!channel_->isWriting())
		{
			channel_->enableWriting();//这里一一定要注册channel的写事件，否则poller不会给channel通知epollout
		}
	}
}

void TcpConnection::connectEstablished()
{
	setState(kConnected);
	channel_->tie(shared_from_this());
	channel_->enableReading();
	//新连接建立，执行回调
	connectionCallback_(shared_from_this());
}
//关闭连接
void TcpConnection::shutdown()
{
	if(state_==kConnected)
	{
		setState(kDisconnecting);
		loop_->runInLoop(
			std::bind(&TcpConnection::shutdownInLoop,this)
				);

	}
}

void TcpConnection::shutdownInLoop()
{
	if(!channel_->isWriting())
	{
		socket_->shutdownwrite();
	}
}
void TcpConnection::connectDestroyed()
{
	if(state_==kConnected)
	{
		setState(kDisconnected);
		channel_->disableALL();
		connectionCallback_(shared_from_this());
	}
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int saveErrno=0;
    ssize_t n=inputBuffer_.readFd(channel_->fd(),&saveErrno);
    if(n>0)
    {
        messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);   
    }
    else if(n==0)
    {
        handleClose();    
    }
    else
    {
        errno=saveErrno;
        LOG_ERROR("TcpConnection::handleRead");
        handleError();    
    }

}
void TcpConnection::handleWrite()
{
    if(channel_->isWriting())
    {
        int saveErrno=0;
        ssize_t n=outputBuffer_.writeFd(channel_->fd(),&saveErrno);   
    }
}
void TcpConnection::handleClose()
{
	LOG_INFO("fd=%d state=%d \n", channel_->fd(),(int)state_);
	setState(kDisconnected);
	channel_->disableALL();

	TcpConnectionPtr connPtr(shared_from_this());
	connectionCallback_(connPtr);//执行关闭连接的回调
	closeCallback_(connPtr);
}
void TcpConnection::handleError()
{
	int optval;
	socklen_t optlen=sizeof optval;
	int err=0;
	if(::getsockopt(channel_->fd(),SOL_SOCKET,SO_ERROR,&optval,&optlen)<0)
	{
		err = errno;
	}
	else
	{
		err =optval;
	}

	LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d \n",name_.c_str(),err);
}

