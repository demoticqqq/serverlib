//
// Created by huangw on 22-9-30.
//

#ifndef SERVERLIB_NONCOPYABLE_H
#define SERVERLIB_NONCOPYABLE_H
class noncopyable
{
public:
    noncopyable(const noncopyable & )=delete;
    noncopyable &operator=(const noncopyable &) = delete;
protected:
    noncopyable()=default;
    ~noncopyable()=default;
};
#endif //SERVERLIB_NONCOPYABLE_H
