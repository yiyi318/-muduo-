#include<string>
#include<functional>
#include"eventloop.h"
#include"tcpserver.h"
#include"tcpconnection.h"
class echoserver
{
public:
    echoserver(EventLoop *Loop,
                const InetAddress &addr,
                const std::string &name)
                :server_(Loop,addr,name)
                ,loop_(Loop)
    {
        //CALLBACK
                            server_.setConnectionCallback(std::bind(&echoserver::onConnection,this,std::placeholders::_1));    
    server_.setMessageCallback(
        std::bind(&echoserver::onMessage,this,
                std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    server_.setThreadNum(3);}
void start(){ server_.start();}

private:
    void onConnection(const TcpConnectionPtr &conn)
    {
        if(conn->connected()){}
        else{}
        
    }
    
    void onMessage(const TcpConnectionPtr & conn,Buffer *buf,Timestamp time)
    {
        std::string msg =buf->retrieveAllAsString();
        conn->send(msg);
        conn->shutdown();   
    }
    EventLoop * loop_;
    TcpServer server_;

};


int main()
{
    EventLoop loop;
    InetAddress addr(8000);
    echoserver server(&loop,addr,"name");
    server.start();
    loop.loop();
    return 0;

}
