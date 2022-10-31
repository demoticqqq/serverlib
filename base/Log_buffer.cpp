//
// Created by huangw on 22-10-29.
//

#include "Log_buffer.h"


template<int SIZE>
const char* FixedBuffer<SIZE>::debugString()
{
    *cur_ = '\0';
    return data_;
}