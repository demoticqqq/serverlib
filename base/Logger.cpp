//
// Created by huangw on 22-10-29.
//
#include "Logger.h"
#include "CurrentThread.h"

#include <stdarg.h>
#include <stdio.h>

__thread char t_time[64];
__thread time_t t_lastSecond;

Logger::LogLevel initLogLevel()
{
    if(::getenv("SERVER_LOG_TRACE"))
        return Logger::TRACE;
    else if(::getenv("SERVER_LOG_DEBUG"))
        return Logger::DEBUG;
    else
        return Logger::INFO;
}

Logger::LogLevel g_logLevel = initLogLevel();

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
        {
          "TRACE",
          "DEBUF",
          "INFO" ,
          "WARN" ,
          "ERROR",
          "FATAL",
        };

void defaultOutput(const char* msg, int len)
{
    size_t n = fwrite(msg,1,len,stdout);
}
void defaultFlush()
{
    fflush(stdout);
}
Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

Logger::Logger(Logger::SourceFile file, int line)
    : time_(Timestamp::now())
    , logResult()
    , level_(INFO)
    , line_(line)
    , basename_(file)
{
    formatTime();
    logResult.append(std::to_string(CurrentThread::tid()) + " ");
    logResult.append(LogLevelName[level_]);
    logResult.append(" ");
}
Logger::Logger(Logger::SourceFile file, int line, Logger::LogLevel level)
    : time_(Timestamp::now())
    , logResult()
    , level_(level)
    , line_(line)
    , basename_(file)
{
    formatTime();
    logResult.append(std::to_string(CurrentThread::tid()) + " ");
    logResult.append(LogLevelName[level_]);
    logResult.append(" ");
}
Logger::Logger(Logger::SourceFile file, int line, Logger::LogLevel level, const char* func)
        : time_(Timestamp::now())
        , logResult()
        , level_(level)
        , line_(line)
        , basename_(file)
{
    formatTime();
    logResult.append(std::to_string(CurrentThread::tid()) + " ");
    logResult.append(LogLevelName[level_]);
    logResult.append(" ");
    logResult.append(func);
}
Logger::Logger(Logger::SourceFile file, int line, bool toAbort)
        : time_(Timestamp::now())
        , logResult()
        , level_(toAbort ? FATAL : ERROR)
        , line_(line)
        , basename_(file)
{
    formatTime();
    logResult.append(std::to_string(CurrentThread::tid()) + " ");
    logResult.append(LogLevelName[level_]);
    logResult.append(" ");
}
void Logger::formatTime()
{
    int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(microSecondsSinceEpoch & Timestamp::kMicroSecondsPerSecond);
    if(seconds != t_lastSecond)
    {
        tm *tm_time = localtime(&seconds);

        snprintf(t_time,sizeof t_time,"%4d/%02d/%02d %02d:%02d:%02d",
                 tm_time->tm_year+1900,tm_time->tm_mon+1,tm_time->tm_mday,
                 tm_time->tm_hour,tm_time->tm_min,tm_time->tm_sec);
    }
    logResult.append(t_time);
    char buf[16];
    snprintf(buf,sizeof buf,".%06d ",microseconds);
    logResult.append(buf);
}

void Logger::writeLog(const char *fmt, ...)
{
    if(!fmt)
    {
        return;
    }
    std::string str;
    va_list ap;
    va_start(ap,fmt);
    if(fmt != nullptr)
    {
        char fmtBuf[1024];
        int writen_n = vsnprintf(fmtBuf,sizeof fmtBuf,fmt,ap);
        if(writen_n > 0)
        {
            str = fmtBuf;
        }
        if(str.empty())
        {
            return;
        }
    }
    va_end(ap);
    str.erase(str.end()-1);
    logResult.append(str);
}

void Logger::finish()
{
    logResult.append(" - ");
    logResult.append(basename_.data_);
    logResult.append(":");
    logResult.append(std::to_string(line_) + '\n');
}

Logger::~Logger()
{
    finish();
    g_output(logResult.c_str(),logResult.size());
    if(level_ == FATAL)
    {
        g_flush();
        abort();
    }
}

void Logger::setLogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}

void Logger::setOutput(Logger::OutputFunc out)
{
    g_output = out;
}

void Logger::setFlush(Logger::FlushFunc flush)
{
    g_flush = flush;
}