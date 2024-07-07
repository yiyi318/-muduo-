#pragma once
#include<vector>
#include"noncopyable.h"
#include<functional>
#include"timestamp.h"
#include<memory>
#include<mutex>
#include<atomic>
#include"currentthread.h"
#include"poller.h"
#include"channel.h"

class Poller;
class Channel;


class EventLoop : noncopyable
{
public:
	using Functor =std::function<void()>;
	EventLoop();
	~EventLoop();
	//开启事件循环
	void loop();
	//退出事件循环
	void quit();
	Timestamp pollReturnTime()const {return pollReturnTime_;}
	//在当前loop执行cd
	void runInLoop(Functor cb);
	//把cd放入队列
	void queueInLoop(Functor cb);
	//唤醒loop所在的线程
	void wakeup();

	void updateChannel(Channel *channel);
	void removeChannel(Channel *channel);
	void hasChannel(Channel *channel);
	//判断eventloop对象是否在在即的线程中
	bool isInLoopThread()const { return threadId_== CurrentThread::tid(); }


private:
	void handleRead();
	void doPendingFunctors();

	using ChannelList=std::vector<Channel*>;

	std::atomic_bool looping_;//原子操作，通过CAS实现的
	std::atomic_bool quit_;//标志退出loop循环
	std::atomic_bool callingPendingFunctors_;//标识当前loop是否有需要执行的回调操作
	const pid_t threadId_;//记录当前loop所在线程的id；
	Timestamp pollReturnTime_;//poller返回发生时间的channels的时间点
	std::unique_ptr<Poller> poller_;

	int wakeupFd_;//主要作用，当mianLoop获取一个新用户的channel，通过轮算法选择一个subloop，通过改成员唤醒loop.
	std::unique_ptr<Channel> wakeupChannel_;

	ChannelList activeChannels_;
	Channel * currentActiveChannel_;

	std::vector<Functor> pendingFunctors_;//存储loop需要执行的所有的回调操作
	std::mutex mutex_;//互斥锁，用来保护上面vector容器的线程安全操作
};
