#include"eventloopthread.h"
#include"eventloop.h"
EventloopThread::EventloopThread(const ThreadInitCallback &cb,const std::string &name)
    :loop_(nullptr)
    ,exiting_(false)
    ,thread_(std::bind(&EventloopThread::threadFunc, this),name)
    ,mutex_()
    ,cond_()
    ,callback_(cb)
{
    
}
	


EventloopThread::~EventloopThread()
{
    exiting_=true;
    if(loop_!=nullptr)
    {
        loop_->quit();
        thread_.join();    
    }

}
EventLoop * EventloopThread::startloop()
{
    thread_.start();//run the below thread

    EventLoop *loop=nullptr;
    {
        std::unique_lock<std::mutex>   lock(mutex_);
        while( loop_==nullptr){
            cond_.wait(lock);
        }
        loop =loop_; 
    }
    return loop;
}
//the below method run in new thread
void EventloopThread::threadFunc()
{
    EventLoop loop;//make a single thread map to the above method,one loop per thread
    if(callback_)
    {
        callback_(&loop);    
    }
    {
        std::unique_lock<std::mutex>   lock(mutex_);
        loop_=&loop;
        cond_.notify_one();
 
    }

    loop.loop();//Eventloop loop=>poller.poll
    std::unique_lock<std::mutex> lock(mutex_);
    loop_=nullptr;
    
}
