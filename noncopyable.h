#pragma once


/*noncopyable被继承以后，派生类对象可以正常的构造析构函数
 * ，但是无法进行拷贝，赋值操作*/
class noncopyable
{
public:
	noncopyable(const noncopyable&)=delete;
	noncopyable& operator=(const noncopyable&) =delete;
protected:
	noncopyable()=default;
	~noncopyable()=default;

};
	
