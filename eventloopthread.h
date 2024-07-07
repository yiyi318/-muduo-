#pragma once
#include<string>
#include"noncopyable.h"
#include"thread.h"
#include<mutex>
#include<functional>
#include<condition_variable>

class EventLoop;

class EventloopThread:noncopyable
{
public:
	using ThreadInitCallback =std::function<void(EventLoop*)>;
    EventloopThread(const ThreadInitCallback &cb=ThreadInitCallback(),const std::string &name=std::string());
	
    ~EventloopThread();
    EventLoop * startloop();

private:
    void threadFunc();

	EventLoop *loop_;
	bool exiting_;
	Thread thread_;
	std::mutex mutex_;
	std::condition_variable cond_;
    ThreadInitCallback callback_;	    

};
