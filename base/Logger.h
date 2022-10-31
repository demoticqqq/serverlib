//
// Created by huangw on 22-10-29.
//

#ifndef SERVERLIB_LOGGER_H
#define SERVERLIB_LOGGER_H

#include <stdint.h>
#include <string>
#include <cstring>
#include <functional>

#include "Log_buffer.h"
#include "Timestamp.h"

class Logger
{
public:
    using Buffer = FixedBuffer<kSmallBuffer>;
    enum LogLevel
    {
        TRACE = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS
    };
    class SourceFile
    {
    public:
        template<int N>
        SourceFile(const char (&arr)[N])
            : data_(arr)
            , size_(N-1)
        {
            const char* slash = strrchr(data_,'/');
            if(slash)
            {
                data_ = slash + 1;
                size_ -= static_cast<int>(data_ - arr);
            }
        }
        explicit SourceFile(const char* filename)
            :data_(filename)
        {
            const char* slash = strrchr(filename,'/');
            if(slash)
            {
                data_ = slash + 1;
            }
            size_ = static_cast<int>(strlen(data_));
        }
        const char* data_;
        int size_;
    };
    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char* func);
    Logger(SourceFile file, int line, bool toAbort);
    ~Logger();
    void writeLog(const char* fmt,...);

    static Logger::LogLevel logLevel();
    static void setLogLevel(Logger::LogLevel level);

    using OutputFunc = std::function<void(const char*, int)>;
    using FlushFunc = std::function<void()>;

    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc);

private:
    void formatTime();
    void finish();

    std::string logResult;
    Timestamp time_;
    Logger::LogLevel level_;
    int line_;
    SourceFile basename_;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel()
{
    return g_logLevel;
}

#define LOG_TRACE(fmt,args...) if (Logger::logLevel() <= Logger::TRACE) \
    Logger(__FILE__, __LINE__, Logger::TRACE, __FUNCTION__).writeLog(fmt,##args)
#define LOG_DEBUG(fmt,args...) if (Logger::logLevel() <= Logger::DEBUG) \
    Logger(__FILE__, __LINE__, Logger::DEBUG, __FUNCTION__).writeLog(fmt,##args)
#define LOG_INFO(fmt,args...) if (Logger::logLevel() <= Logger::INFO) \
    Logger(__FILE__, __LINE__).writeLog(fmt,##args)
#define LOG_WARN(fmt,args...) Logger(__FILE__, __LINE__, Logger::WARN).writeLog(fmt,##args)
#define LOG_ERROR(fmt,args...) Logger(__FILE__, __LINE__, Logger::ERROR).writeLog(fmt,##args)
#define LOG_FATAL(fmt,args...) Logger(__FILE__, __LINE__, Logger::FATAL).writeLog(fmt,##args)
#define LOG_SYSERR(fmt,args...) ogger(__FILE__, __LINE__, false).writeLog(fmt,##args)
#define LOG_SYSFATAL(fmt,args...) Logger(__FILE__, __LINE__, true).writeLog(fmt,##args)

;

#endif //SERVERLIB_LOGGER_H
