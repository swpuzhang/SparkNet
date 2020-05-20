#ifndef TIMEHELPER_H
#define TIMEHELPER_H
#include <string>
#include "date/date.h"

using namespace date;
using namespace std::chrono;

template<typename _Clock, typename _Dur>
std::string GetDayStr(time_point< _Clock, _Dur>  ttime)
{
    std::stringstream strstr;
    strstr << floor<days>(ttime);
    return strstr.str();
}
#endif // !TIMEHELPER_H

