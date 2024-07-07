#pragma once
#include"noncopyable.h"
#include<string>
#define LOG_INFO(LogmsgFormat,...)\
        do\
        {\
                Logger &logger =Logger::instance(); \
                logger.setLoglevel(INFO); \
                char buf[1024]={0}; \
                snprintf(buf,1024,LogmsgFormat,##__VA_ARGS__); \
                logger.log(buf); \
        }while(0)


#define LOG_ERROR(LogmsgFormat,...)\
        do\
        {\
                Logger &logger =Logger::instance(); \
                logger.setLoglevel(ERROR); \
                char buf[1024]={0}; \
                snprintf(buf,1024,LogmsgFormat,##__VA_ARGS__); \
                logger.log(buf); \
        }while(0)

#define LOG_FATAL(LogmsgFormat,...)\
        do\
        {\
                Logger &logger =Logger::instance(); \
                logger.setLoglevel(FATAL); \
                char buf[1024]={0}; \
                snprintf(buf,1024,LogmsgFormat,##__VA_ARGS__); \
                logger.log(buf); \
		exit(-1); \
        }while(0)


#ifdef MUDEBUG
#define LOG_DEBUG(LogmsgFormat,...)\
        do\
        {\
                Logger &logger =Logger::instance(); \
                logger.setLoglevel(DEBUG); \
                char buf[1024]={0}; \
                snprintf(buf,1024,LogmsgFormat,##__VA_ARGS__); \
                logger.log(buf); \
        }while(0)
#else
	#define LOG_DEBUG(LogmsgFormat,...)
#endif
//定义日志的级别 INFO   ERROR   FATAL  DEBUG  
enum LogLevel 
{
	INFO,
	ERROR,
	FATAL,
	DEBUG,
};

//输出一个日志类
//单例模式
class Logger:noncopyable
{
public:
	//获取唯一的实例对象
	static Logger& instance();
	//设置日志级别
	void setLoglevel(int level);
	//写日志
	void log(std::string msg);
private:
	int loglevel_;
	Logger(){}

};
