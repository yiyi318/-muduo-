#include"poller.h"
#include"channel.h"

Poller::Poller(EventLoop *Loop)
	:ownerLoop_(Loop)
{
}

        //判断参数channel是否在当前poller当中
bool Poller::hasChannel(Channel *channel) const
{
	auto it=channels_.find(channel->fd());
	return it!= channels_.end()&&it->second==channel;
}

        //eventloop 可以通过该接口获取默认的IO复用的具体实现，获取一个具体的实例

