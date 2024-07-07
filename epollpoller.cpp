#include"epollpoller.h"
#include"Logger.h"
#include<errno.h>
#include <cstring>
#include <unistd.h>
#include<strings.h>
#include "channel.h"
#include"poller.h"
const int kNew=-1;//channel未添加到数组中
const int kAdded=1;//channel已经添加到poller
const int kDeleted=2;//channel从poller中删除

class channel;

EPollerPoller::EPollerPoller(EventLoop * Loop)
	:Poller(Loop)
	,epollfd_(::epoll_create1(EPOLL_CLOEXEC))
	,events_(kInitEventListSize)
{
	if(epollfd_<0){
		LOG_FATAL("epoll_create error:%d \n",errno);
	}
}
EPollerPoller::~EPollerPoller(){
	::close(epollfd_);
}

Timestamp EPollerPoller::poll(int timeoutMs, ChannelList * activeChanneLs){
	//实际上应该用LOG_DEBUG输出日志更为合理
	LOG_INFO("func=%s =>fd total count:%ld \n",__FUNCTION__,channels_.size());

	int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()),timeoutMs);
    int saveErrno=errno;
	Timestamp now(Timestamp::now());

	if(numEvents > 0){
		LOG_INFO("%d events happened \n",numEvents);
		fillActiveChannels(numEvents,activeChanneLs);
		if(numEvents ==events_.size())
		{
			events_.resize(events_.size() *2);
		}
	}
	else if(numEvents == 0)
	{
		LOG_DEBUG("%s timeout! \n", __FUNCTION__);
	}
	else
	{
		if(saveErrno !=EINTR)
		{
			errno =saveErrno;
			LOG_ERROR("EPollPoller::poll() err!");
		}
	}
	return now;
}

void EPollerPoller::updateChannel(Channel * channel) 
{
	const int index=channel->index();
	LOG_INFO("func=%s => fd=%d events=%d index=%d \n",__FUNCTION__,channel->fd(),channel->events(),index);
	if(index==kNew ||index ==kDeleted)
	{
		if(index==kNew){
			int fd=channel->fd();
			channels_[fd]=channel;
		
		}
		channel->set_index(kAdded);
		update(EPOLL_CTL_ADD,channel);
	}
	else//channel已经在poller上注册过了
	{
		int fd=channel->fd();
		if(channel->isNoneEvent())
		{
			update(EPOLL_CTL_DEL,channel);
			channel->set_index(kDeleted);
		}
		else{
			update(EPOLL_CTL_MOD,channel);
		}
	}
}

void EPollerPoller::removeChannel(Channel *channel)
{
	int fd=channel->fd();

	channels_.erase(fd);
	LOG_INFO("func=%s => fd=%d \n",__FUNCTION__,fd);

	int index=channel->index();
	if(index==kAdded)
	{
		update(EPOLL_CTL_DEL,channel);
	}
	channel->set_index(kNew);
}

//填写活跃的连接
void EPollerPoller::fillActiveChannels(int numEvents,ChannelList *activeChannels) const
{
	for(int i=0;i<numEvents;i++)
	{
		Channel * channel =static_cast<Channel*>(events_[i].data.ptr);
		channel->set_revents(events_[i].events);
		activeChannels->push_back(channel);
	}
}
        //更新channel的通道
void EPollerPoller::update(int operation,Channel *channel)
{
	epoll_event event;
	bzero(&event,sizeof event);

	int fd=channel->fd();

	event.events=channel->events();
	event.data.fd=fd;
	event.data.ptr=channel;

	if(::epoll_ctl(epollfd_,operation,fd,&event)<0){
		if(operation==EPOLL_CTL_DEL){
			LOG_ERROR("epoll_ctl del error:%d\n",errno);
		}
		else
		{
			LOG_FATAL("epoll_ctl add/mod error:%d\n",errno);
		}
	}

}
