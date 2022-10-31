//
// Created by huangw on 22-10-30.
//
#include "FileUtil.h"
#include "LogFile.h"
#include <unistd.h>

LogFile::LogFile(const std::string &basename, off_t rollSize, bool threadSafe, int flushInterval, int checkEveryN)
    : basename_(basename)
    , rollSize_(rollSize)
    , flushInterval_(flushInterval)
    , checkEveryN_(checkEveryN)
    , count_(0)
    , mutex_(threadSafe ? new std::mutex : NULL)
    , startOfPeriod_(0)
    , lastFlush_(0)
    , lastRoll_(0)
{
    rollFile();
}
LogFile::~LogFile() = default;

void LogFile::append(const char *logline, int len)
{
    if(mutex_)
    {
        std::unique_lock<std::mutex>lk(*mutex_);
        append_unlocked(logline,len);
    }
    else
    {
        append_unlocked(logline,len);
    }
}

void LogFile::flush()
{
    if(mutex_)
    {
        std::unique_lock<std::mutex>lk(*mutex_);
        file_->flush();
    }
    else
    {
        file_->flush();
    }
}

void LogFile::append_unlocked(const char *logline, int len)
{
    file_->append(logline,len);
    if(file_->writtenBytes() > rollSize_)
    {
        rollFile();
    }
    else
    {
        ++count_;
        if(count_ >= checkEveryN_)
        {
            count_ = 0;
            time_t now = time(nullptr);
            time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_; //日期对齐
            if(thisPeriod_ != startOfPeriod_)
            {
                rollFile();
            }
            else if(now - lastFlush_ > flushInterval_)
            {
                lastFlush_ = now;
                file_->flush();
            }
        }
    }
}

bool LogFile::rollFile()
{
    time_t now = 0;
    std::string filename = getLogFileName(basename_,&now);
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_; //start对齐至零点时刻
    if(now > lastRoll_)
    {
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new AppendFile(filename));
        return true;
    }
    return false;
}

std::string LogFile::getLogFileName(const std::string &basename, time_t *now)
{
    std::string filename;
    filename.reserve(basename.size()+64);
    filename = basename;
    char timebuf[32];
    tm tm;
    *now = time(nullptr);
    localtime_r(now,&tm);
    strftime(timebuf,sizeof timebuf,".%Y%m%d-%H%M%S.",&tm);
    filename += timebuf;
    char hostnamebuf[256];
    std::string hostname;
    if(gethostname(hostnamebuf,sizeof hostnamebuf) == 0)
    {
        hostnamebuf[sizeof(hostnamebuf) - 1] = '\0';
        hostname = hostnamebuf;
    }
    else
    {
        hostname = "unknownhost";
    }
    filename += hostname;

    char pidbuf[32];
    snprintf(pidbuf,sizeof pidbuf,".%d",getpid());
    filename += pidbuf;
    filename += ".log";

    return filename;
}