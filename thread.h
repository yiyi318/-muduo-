#pragma once
#include"noncopyable.h"
#include<functional>
#include<thread>
#include<memory>
#include<unistd.h>
#include<atomic>
#include"currentthread.h"
#include<string>
#include<cstdio>

class Thread:public noncopyable
{
public:
	using ThreadFunc =std::function<void()>;
	explicit Thread(ThreadFunc,const std::string &name =std::string());
	~Thread();
	void start();
	void join();

	bool started() const {return started_;}
	pid_t tid() const {return tid_;}
	const std::string & name() const {return name_;}
    void setDefaultName();

	static int numCreated() { return numCreated_; }
private:
	bool started_;
	bool joined_;
	std::shared_ptr<std::thread> thread_;
	//这里不直接使用thread类初始化对象，是因为要控制thread开启的时间
	pid_t tid_;
	ThreadFunc func_;
	std::string name_;
	static std::atomic_int32_t numCreated_;	
	
};
