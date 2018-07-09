#ifndef TIMER_H
#define TIMER_H

#define DBG_TIMER

#ifdef DBG_TIMER

#include "time.h"
#include <iostream>

#define FCN_TIMER(function) \
{ \
clock_t beg = clock(); \
function \
clock_t end = clock(); \
std::cout << "+ Elapsed: " << (end-beg)/CLOCKS_PER_SEC \
          << " secs in Fn [" << __FUNCTION__ << "] at {" \
          << #function << "}" << std::endl; \
} \

#define TIMER_START() \
  clock_t clock_beg = clock(); \
  long clock_beg_line = __LINE__; \

#define TIMER_STOP() \
  clock_t clock_end = clock(); \
  long clock_end_line = __LINE__; \
  std::cout << "+ Elapsed time: " << (clock_end-clock_beg)/CLOCKS_PER_SEC \
            << " secs in Fn [" << __FUNCTION__ << "] at Ln {" \
            << clock_beg_line << " - " << clock_end_line << "}" << std::endl; \

#define TIMER_STOP_STR(str) \
  clock_t clock_end = clock(); \
  long clock_end_line = __LINE__; \
  std::cout << "+ Elapsed time: " << (clock_end-clock_beg)/CLOCKS_PER_SEC \
            << " secs in Fn [" << __FUNCTION__ << "] at Ln {" \
            << clock_beg_line << " - " << clock_end_line << "}  " \
            << str << std::endl; \

#else

#define FCN_TIMER(function) function
#define TIMER_START()
#define TIMER_STOP()
#define TIMER_STOP_STR(str)

#endif


#endif // TIMER_H
