#ifndef DEBUG_H
#define DEBUG_H

#define DBG_PRINT

#ifdef DBG_PRINT

#include "stdio.h"

#define PRINT(cformat, ...) \
  printf(cformat, __VA_ARGS__); \

#define PRINTA(cformat, ...) \
  printf("- File [%s], Fn {%s}, Ln %d: ", __FILE__, __FUNCTION__, __LINE__); \
  printf(cformat, __VA_ARGS__); \

#else

#define PRINT(cformat, ...)
#define PRINTA(cformat, ...)

#endif

#endif // DEBUG_H
