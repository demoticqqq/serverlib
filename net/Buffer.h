//
// Created by huangw on 22-10-13.
//

#ifndef SERVERLIB_BUFFER_H
#define SERVERLIB_BUFFER_H
#include <vector>
#include <string>

class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize)
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend)
    {
    }

    size_t readableBytes() const {return writerIndex_ - readerIndex_;}
    size_t writableBytes() const {return buffer_.size() - writerIndex_;}
    size_t prependableBytes() const {return readerIndex_;}

    //返回缓冲区中，可读数据的起始地址
    const char* peek() const {return begin() + readerIndex_;}
    //返回缓存区中，可写的起始位置
    char* beginWrite() {return begin() + writerIndex_;}
    const char* beginWrite() const {return begin() + writerIndex_;}

    void retrieve(size_t len)
    {
        if(len < readableBytes())  //说明缓存区中数据没有读完
        {
            readerIndex_ += len;
        }
        else   //说明读完了，重置read和write下标
        {
            retrieveAll();
        }
    }

    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    std::string retrieveAllAsString() { return retrieveAsString(readableBytes());}
    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(),len);  //从缓存区读取数据至string中
        retrieve(len);  //读完后，执行复位操作
        return result;
    }
    //确认可写空间足够，不够的话需要扩容
    void ensureWritableBytes(size_t len)
    {
        if(writableBytes() < len)
        {
            makeSpace(len);
        }
    }

    void append(const char* data,size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data,data+len,beginWrite());
        hasWritten(len);
    }

    void hasWritten(size_t len)
    {
        writerIndex_ += len;
    }

    size_t internalCapacity() const
    {
        return buffer_.capacity();
    }

    void swap(Buffer& other)
    {
        buffer_.swap(other.buffer_);
        std::swap(readerIndex_,other.readerIndex_);
        std::swap(writerIndex_,other.writerIndex_);
    }

    ssize_t readFd(int fd,int *saveErrno);
    ssize_t writeFd(int fd,int *saveErrno);

private:
    char *begin() {return buffer_.data();}
    const char *begin() const {return buffer_.data();}
    void makeSpace(size_t len)
    {
        /**
         * | kCheapPrepend |xxx| reader | writer |                     // xxx标示reader中已读的部分
         * | kCheapPrepend | reader ｜          len          |
         **/
        size_t readable = readableBytes(); // readable = reader的长度
        std::copy(begin() + readerIndex_,
                  begin() + writerIndex_,  // 把这一部分数据拷贝到begin+kCheapPrepend起始处
                  begin() + kCheapPrepend);
        readerIndex_ = kCheapPrepend;
        writerIndex_ = readerIndex_ + readable;
        if (writableBytes() < len ) // 也就是说 len > xxx + writer的部分
        {
            buffer_.resize(writerIndex_ + len);
        }

    }
private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    //static const char kCRLF[];
};


#endif //SERVERLIB_BUFFER_H
