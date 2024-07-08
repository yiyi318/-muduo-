#include"Logger.h"
#include<iostream>

Logger& Logger::instance()
{
	static Logger logger;
	return logger;
}

void Logger::setLoglevel(int level)
{
	loglevel_=level;

}



void Logger::log(std::string msg)
{
	switch(loglevel_)
	{
	case INFO:
		std::cout<<"[INFO]";
		break;
	case ERROR:
		std::cout<<"[ERROR]";
		break;
	case FATAL:
		std::cout<<"[FATAL]";
		break;
	case DEBUG:
		std::cout<<"[DEBUG]";
		break;
	default:
		break;
	}
	//打印事件和msg
	
//打印时间和msg;
	std::cout<<"print time" <<" : "<<msg<<std::endl;

}
