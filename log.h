#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#define NO_LOG          0x00
#define ERROR_LEVEL     0x01
#define INFO_LEVEL      0x02
#define DEBUG_LEVEL     0x03
#define TRACE_LEVEL     0x04

#define LOG_LEVEL TRACE_LEVEL
#ifndef LOG_LEVEL
#define LOG_LEVEL INFO_LEVEL
#endif

#if LOG_LEVEL >= TRACE_LEVEL
#define LOG_TRACE(...) do { printf(__VA_ARGS__); } while (0)
#else
#define LOG_TRACE(...)
#endif

#if LOG_LEVEL >= DEBUG_LEVEL
#define LOG_DEBUG(...) do { printf(__VA_ARGS__); } while (0)
#else
#define LOG_DEBUG(...)
#endif

#if LOG_LEVEL >= INFO_LEVEL
#define LOG_INFO(...) do { printf(__VA_ARGS__); } while (0)
#else
#define LOG_INFO(...)
#endif

#if LOG_LEVEL >= ERROR_LEVEL
#define LOG_ERROR(...) do { printf(__VA_ARGS__); } while (0)
#else
#define LOG_ERROR(...)
#endif

#define LOG(...) do { printf(__VA_ARGS__); } while (0)

#endif // LOG_H
