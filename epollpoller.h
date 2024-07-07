#pragma once

#include"poller.h"
#include<vector>
#include<sys/epoll.h>
#include"timestamp.h"


/**/

class EPollerPoller:public Poller

{
public:

	EPollerPoller(EventLoop * Loop);
	~EPollerPoller() override;
	//重写基类poller的抽象方法
	Timestamp poll(int timeoutMs, ChannelList * activeChanneLs) override;

	void updateChannel(Channel * channel) override;
	
	void removeChannel(Channel *channel) override;
private:
	static const int kInitEventListSize =16;
	//填写活跃的连接
	void fillActiveChannels(int numEvents,ChannelList *activeChannels) const;
	//更新channel的通道
	void update(int operation,Channel *channel);
	using Eventlist =std::vector<epoll_event>;
	int epollfd_;
	Eventlist events_;

};
