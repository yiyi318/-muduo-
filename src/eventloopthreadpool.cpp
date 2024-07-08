#include"eventloopthreadpool.h"
#include"eventloopthread.h"

EventloopThreadPool::EventloopThreadPool(EventLoop *baseLoop,const std::string&nameArg)
    :baseLoop_(baseLoop)
    ,name_(nameArg)
    ,started_(false)
    ,numThreads_(0)
    ,next_(0)
{}

EventloopThreadPool::~EventloopThreadPool()
{}

void EventloopThreadPool::start(const ThreadInitCallback &ch)
{
    started_=true;
    for(int i=0;i<numThreads_;i++){// make pool,init numthreads pool
           char buf[name_.size()+32];
           snprintf(buf,sizeof buf,"%s%d",name_.c_str(),i); 
           EventloopThread *t=new EventloopThread(ch,buf);
           threads_.push_back(std::unique_ptr<EventloopThread>(t)); 
           loops_.push_back(t->startloop());
    }
    //
    if(numThreads_==0 && ch)   
    {
        ch(baseLoop_);
    }
}
//if work in many thread ,baseloop arrange channel ro subloop by lunxun in default
EventLoop * EventloopThreadPool::getNextLoop()
{
    EventLoop *loop=baseLoop_;//get handlerevent by lunxun
    if(!loops_.empty()){
        loop=loops_[next_]; 
        ++next_;
        if(next_>=loops_.size())
        {
            next_=0;        
        }
    }
    return loop;
}

std::vector<EventLoop*> EventloopThreadPool::getAllLoops()
{
    if(loops_.empty())
    {
        return std::vector<EventLoop *> (1,baseLoop_);
    }
    else{
        loops_;    
    }
    

}
