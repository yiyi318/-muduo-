#include"eventloop.h"
#include<unistd.h>
#include<fcntl.h>
#include"Logger.h"
#include<errno.h>
#include<sys/eventfd.h>


__thread EventLoop *t_loopInThisThread = nullptr;

const int kPollTimeMs=10000;

//创建wakeupfd 用来唤醒loop所在线程
int createEventfd()
{
	int evtfd=::eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);

	if(evtfd <0 )
	{
		LOG_FATAL("eventfd error:%d \n",errno);
	}
	return evtfd;
}
EventLoop::EventLoop()
	:looping_(false)
	,quit_(false)
	,callingPendingFunctors_(false)
	,threadId_(CurrentThread::tid())
	,poller_(Poller::newDefaultPoller(this))
	,wakeupFd_(createEventfd())
	,wakeupChannel_(new Channel(this,wakeupFd_))
{
	LOG_DEBUG("EventLoop created %p in thread %d \n",this,threadId_);
	if(t_loopInThisThread)
	{
		LOG_FATAL("Another EventLoop %p exists in this thread %d \n",t_loopInThisThread,threadId_);
	}
	else
	{
		t_loopInThisThread=this;
	}
	wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead,this));
	wakeupChannel_->enableReading();
	//设置wakeup的事件以及发生事件后的回调操作
	
}
EventLoop::~EventLoop(){
	wakeupChannel_->disableALL();
	wakeupChannel_->remove();
	::close(wakeupFd_);
	t_loopInThisThread=nullptr;
}

void EventLoop::handleRead()
{
	uint64_t one=1;
	ssize_t n=read(wakeupFd_,&one,sizeof one);

}


void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
	callingPendingFunctors_=true;
	{
		std::unique_lock<std::mutex> lock(mutex_);
		functors.swap(pendingFunctors_);
	}
	
	for(const Functor &functor:functors)
	{
		functor();//执行当前loop需要执行的回调操作
	}

	callingPendingFunctors_=false;
}



void EventLoop:: loop(){
	looping_=true;
      	quit_=false;

	LOG_INFO("EventLoop %p start looping \n",this);
	
	while(!quit_)
	{
		activeChannels_.clear();
		//监听两类fd 一种是client的fd,一种是wakeupfd
		pollReturnTime_=poller_->poll(kPollTimeMs,&activeChannels_);
		for(Channel *channel:activeChannels_)
		{
			//poller监听哪些channel发生事件了，然后上报给EventLoop,通知channel处理相应的事件
			channel->handleEvent(pollReturnTime_);
		}
		//执行当前eventloop事件循环需要处理的回调操作
		doPendingFunctors();
	
	}
	LOG_INFO("EventLoop %p stop looping.\n",this);
	looping_=false;	
}
        //退出事件循环 1 loop在自己的线程中调用quit 2在非loop的线程中，调用loop的quit
void EventLoop::quit()
{
	quit_=true;
	if(!isInLoopThread())//如果是在其它线程中，调用的quit，在一个subloop中，第哦啊用了mainloop的quit
	{
		wakeup();
	}
}


 void EventLoop::runInLoop(Functor cb)
{
	if(isInLoopThread()){
		cb();
	}
	else//在非loop的线程中，那就需要唤醒loop所在线程，执行cb
	{
		queueInLoop(cb);
	}
}
        //把cd放入队列
 void EventLoop::queueInLoop(Functor cb)
{
	{
		std::unique_lock<std::mutex> lock(mutex_);
		pendingFunctors_.emplace_back(cb);
	}
	//唤醒相应的，需要执行上面回调操作的loop的线程
	if(!isInLoopThread() || callingPendingFunctors_)//这里的callingPendingFunctors
	{
		wakeup();//唤醒loop所在线程
	}
}

//用来唤醒loop所在的线程，向wakeuo_fd写一个数据
void EventLoop::wakeup()
{
	uint64_t one=1;
	ssize_t n=write(wakeupFd_,&one,sizeof one);
	if(n!=sizeof one)
	{
		LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 \n",n);
	}
}

void EventLoop::updateChannel(Channel *channel)
{
	poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel *channel)
{
	poller_->removeChannel(channel);
}
void EventLoop::hasChannel(Channel *channel)
{
	poller_->hasChannel(channel);
}
