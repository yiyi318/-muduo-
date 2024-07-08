#include"poller.h"
#include<stdlib.h>

#include"epollpoller.h"

Poller* Poller::newDefaultPoller(EventLoop * Loop)
{
	if(::getenv("MUDUO_USE_POLL"))
	{
		return nullptr;
	}
	else
	{
		return new EPollerPoller(Loop);
	}
}
