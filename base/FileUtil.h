//
// Created by huangw on 22-10-30.
//

#ifndef SERVERLIB_FILEUTIL_H
#define SERVERLIB_FILEUTIL_H

#include "noncopyable.h"
#include <string>
class AppendFile : noncopyable
{
public:
    explicit AppendFile(std::string filename);

    ~AppendFile();

    void append(const char* logline, size_t len);

    void flush();

    off_t writtenBytes() const { return writtenBytes_; }

private:

    size_t write(const char* logline, size_t len);

    FILE* fp_;
    char buffer_[64*1024];
    off_t writtenBytes_;
};

#endif //SERVERLIB_FILEUTIL_H
