//
// Created by huangw on 22-9-30.
//

#ifndef SERVERLIB_CURRENTTHREAD_H
#define SERVERLIB_CURRENTTHREAD_H

#include <unistd.h>
#include <sys/syscall.h>
namespace CurrentThread {

    extern __thread int t_cachedTid;
    extern __thread const char* t_threadName;
    void cachedTid();

    inline int tid()
    {
        if(__builtin_expect(t_cachedTid==0,0)) //__builtin_expect 是一种底层优化 此语句意思是如果还未获取tid 进入if 通过cacheTid()系统调用获取tid
        {
            cachedTid();
        }
        return t_cachedTid;
    }
    inline const char* name()
    {
        return t_threadName;
    }

    bool isMainThread();



} // CurrentThread

#endif //SERVERLIB_CURRENTTHREAD_H
