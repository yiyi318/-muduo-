#pragma once
#include "noncopyable.h"
#include<vector>
#include<unordered_map>
#include"timestamp.h"
class Channel;
class EventLoop;
//muduo库中多路事件分发器的核心IO复用模块
class Poller: noncopyable
{
public:
	using ChannelList=std::vector<Channel*>;
	Poller(EventLoop *Loop);
	virtual ~Poller()=default;
	//给所有的IO复用保留统一的接口
	virtual Timestamp poll(int timeoutMs,ChannelList *activeChanneLs)=0;
	virtual void updateChannel(Channel* channel)=0;
	virtual void removeChannel(Channel * channel)=0;
	//判断参数channel是否在当前poller当中
	bool hasChannel(Channel *channel) const;
	
	//eventloop 可以通过该接口获取默认的IO复用的具体实现，获取一个具体的实例
	static Poller* newDefaultPoller(EventLoop *Loop);
protected:
	using ChannelMap=std::unordered_map<int,Channel*>;
	ChannelMap channels_;
private:
	EventLoop * ownerLoop_;
};
