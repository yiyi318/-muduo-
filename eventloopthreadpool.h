#pragma once
#include"noncopyable.h"

#include<functional>
#include<string>
#include<memory>
#include<vector>
class EventLoop;
class EventloopThread;

class EventloopThreadPool:noncopyable
{
public:
    using ThreadInitCallback=std::function<void(EventLoop*)>;
    EventloopThreadPool(EventLoop *baseLoop,const std::string&nameArg);
    ~EventloopThreadPool();
    void setThreadNum(int numThreads){numThreads_=numThreads;}
    void start(const ThreadInitCallback &ch=ThreadInitCallback());
    //if work in many thread ,baseloop arrange channel ro subloop by lunxun in default
    EventLoop * getNextLoop();
    std::vector<EventLoop*> getAllLoops();
    bool started() const {return started_;}
    const std::string name() const {return name_;}
private:
    EventLoop *baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventloopThread>> threads_;
    std::vector<EventLoop*> loops_;
    
};
     
