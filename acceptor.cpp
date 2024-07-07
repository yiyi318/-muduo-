#include"acceptor.h"
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include"inetaddress.h"
#include<netinet/tcp.h>
static int createNonblocking()
{
    int sockfd=::socket(AF_INET,SOCK_STREAM |SOCK_NONBLOCK |SOCK_CLOEXEC,IPPROTO_TCP);
    if(sockfd<0)
    {
        LOG_FATAL("%s:%s:%d listen socket create err:%d \n",__FILE__,__FUNCTION__,__LINE__,errno);
    }
}
Acceptor::Acceptor(EventLoop *Loop,const InetAddress & ListenAddr,bool reuseport)
:loop_(Loop)
,acceptSocket_(createNonblocking())
,acceptChannel_(Loop,acceptSocket_.fd())
,listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(ListenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));
}
Acceptor::~Acceptor(){
    acceptChannel_.disableALL();
    acceptChannel_.remove();
}

void Acceptor::listen()
{

    listenning_ =true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
    
}
//liestenfd new person connect
void Acceptor::handleRead()
{
    InetAddress portAddr;
    int connfd=acceptSocket_.accept(&portAddr);
    if(connfd>=0)
    {
         if(newConnectionCallback_)
         {
                newConnectionCallback_(connfd,portAddr);
         }
         else{
                ::close(connfd);
         } 
    }
    else
    {
        LOG_ERROR("%s:%s:%d accept err:%d \n",__FILE__,__FUNCTION__,__LINE__,errno); 
        if(errno==EMFILE)
        {
            LOG_ERROR("%s:%s:%d sockfd reached limit ,errno %d \n",__FILE__,__FUNCTION__,__LINE__,errno);         
        }
    }
}

