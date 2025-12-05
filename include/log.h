/* Simple logging macros controlled by -DDEBUG build flag */
#ifndef MSH_LOG_H
#define MSH_LOG_H

#include <stdio.h>

#ifdef DEBUG
  #define MSH_LOG(fmt, ...) \
    do { fprintf(stderr, "[minishell][%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); } while (0)
#else
  #define MSH_LOG(fmt, ...) do { } while (0)
#endif

#endif