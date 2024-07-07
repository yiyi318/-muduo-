#include"buffer.h"
#include<errno.h>
#include<sys/uio.h>
#include<unistd.h>
/* read data from fd,poller work in LT model
    buffer has size,but read from fd,do not the size of data finally at once
    
*/
ssize_t Buffer::readFd(int fd,int * saveErrno)
{
//the room in zhan
    char extrabuf[65536]={0};//64 k 1k=1024b
    struct iovec vec[2];
    const size_t writeable=writeableBytes();
    vec[0].iov_base=begin()+writeIndex_;
    vec[0].iov_len=writeable;
    vec[1].iov_base=extrabuf;
    vec[1].iov_len=sizeof extrabuf;
    const int iovcnt=(writeable <sizeof extrabuf)? 2:1;
    const ssize_t n=::readv(fd,vec,iovcnt);
    if(n<0)
    {
        *saveErrno=errno;    
    }
    else if(n<=writeable)
    {
          writeIndex_+=n;  
    }
    else//extrabuff writed buf
    {
        writeIndex_=buffer_.size();
        append(extrabuf,n-writeable); //writeIndex_ start to write
               
    }
    return n;
}

ssize_t Buffer::writeFd(int fd,int * saveErrno)
{
    ssize_t n=::write(fd, peek(),readableBytes());
    if(n<=0)
    {
        *saveErrno=errno;    
    }
    return n;
}
