#pragma once
#include<vector>
#include<string>
#include<algorithm>

class Buffer
{
public:
    static const size_t kCheapPrepend=8;
    static const size_t kInitalSize=1024;
    explicit Buffer(size_t initalSize  =kInitalSize)
        :buffer_(kCheapPrepend+initalSize)
        ,readerIndex_(kCheapPrepend)
        ,writeIndex_(kCheapPrepend)
    {}
    size_t readableBytes() const
    {
        return writeIndex_-readerIndex_;    
    }

    size_t writeableBytes() const
    {
        return buffer_.size()-writeIndex_;    
    }
    size_t prependableBytes() const
    {
        return readerIndex_;    
    }
//return buf readable address
    const char* peek() const
    {

        return begin()-readerIndex_;    
    }
    //onmessage string<= buffer
    void retrieve(size_t len){
            if(len<readableBytes())
            {
                  readerIndex_+=len;                          
            }else
            {
                retrieveAll();            
            }
    }
    void retrieveAll()
    {
        readerIndex_=writeIndex_=kCheapPrepend;
    }
    std::string retrieveAllAsString()
    {
           return retrievrAsString(readableBytes());//apply readable data size 
    }
    std::string retrievrAsString(size_t len)
    {
            std::string result(peek(),len);
            retrieve(len);//the above sentence read,this sentence reset 
            return result;
    }
    void ensurewriteableBytes(size_t len)//expand capacity
    {
        if(writeableBytes()<len)
        {
            makeSpace(len);
        } 
    }
    void append(const char * data,size_t len)//palce [data,data+len]data in buf
    {
            ensurewriteableBytes(len);
            std::copy(data,data+len,beginWrite());
            writeIndex_+=len;
    }
    char *beginWrite()
    {
        return begin()+writeIndex_;    
    }
    const char* beginWrite() const
    {
        return begin()+writeIndex_;    
    }
    //from fd read data
    ssize_t readFd(int fd,int * saveErrno);

    ssize_t writeFd(int fd,int * saveErrno);
private:
    char * begin()
    {
        //it.operator()
        return &*buffer_.begin();//vector
            
    }
    const char* begin() const
    {
        return &*buffer_.begin();    
    }

    void makeSpace(size_t len)//expand capacity
    {
        if(writeableBytes()-prependableBytes()<len+kCheapPrepend)
        {
            buffer_.resize(writeIndex_+len);       
        }
        else
        {
            size_t readable=readableBytes();
            std::copy(begin()+readerIndex_,begin()+writeIndex_,begin()+kCheapPrepend);
            readerIndex_=kCheapPrepend;
            writeIndex_=readerIndex_+readable;
        }  
          
    }
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writeIndex_;
};
