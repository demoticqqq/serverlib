//
// Created by huangw on 22-10-29.
//

#ifndef SERVERLIB_SINGLETON_H
#define SERVERLIB_SINGLETON_H

#include <pthread.h>
#include <stdlib.h>

template<typename T>
class Singleton
{
public:
    static T* getInstance()
    {
        pthread_once(&onceControl_,&Singleton::init);
        return value_;
    }
    static void destroy()
    {
        if(value_ != nullptr)
        {
            delete value_;
        }
    }

private:
    Singleton();
    ~Singleton();

    static void init()
    {
        value_ = new T();
        atexit(destroy);
    }

    static T* value_;
    static pthread_once_t onceControl_;
};

template <typename T>
pthread_once_t Singleton<T>::onceControl_ = PTHREAD_ONCE_INIT;

template <typename T> T *Singleton<T>::value_ = nullptr;


#endif //SERVERLIB_SINGLETON_H
