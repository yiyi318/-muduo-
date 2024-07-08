#include"thread.h"
#include<semaphore.h>

std::atomic_int Thread::numCreated_(0);
Thread::Thread(ThreadFunc func, const std::string & name)
	:started_(false)
	,joined_(false)
	,tid_(0)
	,func_(std::move(func))
	,name_(name)
{
	setDefaultName();

}

Thread::~Thread()
{
	if(started_ && !joined_){
		thread_->detach();
	}
}
void Thread::start()//一个thread对象，记录的就是一个新线程
{
	started_=true;
	sem_t sem;
	sem_init(&sem, false, 0);
	thread_=std::shared_ptr<std::thread>(new std::thread([&](){
			//或缺线程的tid值
			tid_=CurrentThread::tid();
			sem_post(&sem);
			func_();//开启一个新线程 			
	}));
	//这里必须等待获取新创建的线程	
	sem_wait(&sem);

}

void Thread::join()
{
	joined_=true;
	thread_->join();
}


void Thread::setDefaultName()
{
	int num = ++numCreated_;
	if(name_.empty()){
		char buf[32]={0};
		snprintf(buf,sizeof buf,"Thread%d",num);
		name_=buf;
	}

}
