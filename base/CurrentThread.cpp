//
// Created by huangw on 22-10-22.
//
#include "CurrentThread.h"

namespace CurrentThread
{
    __thread int t_cachedTid = 0;
    __thread const char* t_threadName = "unknown";
}