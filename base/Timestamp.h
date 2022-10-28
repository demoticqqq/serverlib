//
// Created by huangw on 22-10-1.
//

#ifndef SERVERLIB_TIMESTAMP_H
#define SERVERLIB_TIMESTAMP_H
#include <string>
#include "Logger.h"
class Timestamp
{
public:
    Timestamp()
        :microSecondsSinceEpoch_(0)
    {
    }

    explicit Timestamp(int64_t microSecondsSinceEpochArg)
        :microSecondsSinceEpoch_(microSecondsSinceEpochArg)
    {
    }

    static  Timestamp now();
    std::string toString() const;

    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    int64_t microSecondsSinceEpoch() const {return microSecondsSinceEpoch_;}

    static const int kMicroSecondsPerSecond = 1000*1000;

private:
    int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}


inline double timeDifference(Timestamp high,Timestamp low)
{
    int64_t diff = high.microSecondsSinceEpoch()-low.microSecondsSinceEpoch();
    return static_cast<double>(diff)/Timestamp::kMicroSecondsPerSecond;
}
inline Timestamp addTime(Timestamp timestamp,double seconds){
    int64_t delta = static_cast<int64_t>(seconds*Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch()+delta);
}

#endif //SERVERLIB_TIMESTAMP_H
