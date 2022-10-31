//
// Created by huangw on 22-10-30.
//

#ifndef SERVERLIB_LOGFILE_H
#define SERVERLIB_LOGFILE_H

#include "memory"
#include "noncopyable.h"
#include <string>
#include <mutex>

class AppendFile;

class LogFile : noncopyable
{
public:
    LogFile(const std::string& basename,
            off_t rollSize,
            bool threadSafe = true,
            int flushInterval = 3,
            int checkEveryN = 1024);
    ~LogFile();

    void append(const char* logline, int len);
    void flush();
    bool rollFile();

private:
    void append_unlocked(const char* logline, int len);

    static std::string getLogFileName(const std::string& basename, time_t* now);

    const std::string basename_;       //日志文件名
    const off_t rollSize_;        //日志文件大小达到rollsize就换一个新文件
    const int flushInterval_;     //日志写入间隔时间
    const int checkEveryN_;       //与count_比较，检查是否要日志滚动

    int count_;

    std::unique_ptr<std::mutex>mutex_;
    time_t startOfPeriod_;        //
    time_t lastRoll_;
    time_t lastFlush_;
    std::unique_ptr<AppendFile> file_;

    const static int kRollPerSeconds_ = 60*60*24;
};


#endif //SERVERLIB_LOGFILE_H
