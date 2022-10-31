//
// Created by huangw on 22-10-31.
//
//
// Created by huangw on 22-10-31.
//

#include "serverlib/base/AsyncLogging.h"
#include "serverlib/base/Logger.h"
#include "serverlib/base/Timestamp.h"

#include <sys/resource.h>
#include <unistd.h>

off_t kRollsize = 1*1000*1000;

AsyncLogging* g_asyncLog = nullptr;

void asyncOutput(const char* msg, int len)
{
    g_asyncLog->append(msg, len);
}

void bench(bool longLog)
{
    Logger::setOutput(asyncOutput);
    int cnt = 0;
    const int kBatch = 1000;
    std::string empty = " ";
    std::string longStr(3000,'X');
    longStr += " ";

    for(int t = 0; t < 30; t++)
    {
        Timestamp start = Timestamp::now();
        for (int i = 0; i < kBatch; ++i)
        {
            LOG_INFO("Hello 0123456789 abcdeffghijklmnopqrstuvwxyz %s %ld",(longLog?longStr:empty).c_str(),cnt);
            cnt++;
        }
        Timestamp end = Timestamp::now();
        printf("%f\n", timeDifference(end,start)*Timestamp::kMicroSecondsPerSecond / kBatch);
        timespec ts = {0,500*1000*1000};
        nanosleep(&ts, nullptr);
    }
}

int main(int argc,char* argv[])
{
    size_t kOneGB = 1000*1024*1024;
    rlimit rl = {2*kOneGB,2*kOneGB};
    setrlimit(RLIMIT_AS,&rl);

    printf("pid = %d\n",getpid());

    char name[256] = {'\0'};
    stpncpy(name,argv[0],sizeof name -1);
    AsyncLogging log(::basename(name),kRollsize);
    log.start();
    g_asyncLog = &log;

    bool longLog = argc > 1;
    bench(longLog);
}