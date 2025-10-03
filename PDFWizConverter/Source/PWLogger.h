#pragma once
#include <abi/abi_Logger.h>

#ifdef _MSC_VER
    #define PW_TRACE(...)     LOGGER_TRACE((__VA_ARGS__), __builtin_FILE(), __builtin_FUNCSIG(), __builtin_LINE())
    #define PW_DEBUG(...)     LOGGER_DEBUG((__VA_ARGS__), __builtin_FILE(), __builtin_FUNCSIG(), __builtin_LINE())
    #define PW_INFO(...)      LOGGER_INFO((__VA_ARGS__), __builtin_FILE(), __builtin_FUNCSIG(), __builtin_LINE())
    #define PW_WARN(...)      LOGGER_WARN((__VA_ARGS__), __builtin_FILE(), __builtin_FUNCSIG(), __builtin_LINE())
    #define PW_ERROR(...)     LOGGER_ERROR((__VA_ARGS__), __builtin_FILE(), __builtin_FUNCSIG(), __builtin_LINE())
    #define PW_CRITICAL(...)  LOGGER_CRITICAL((__VA_ARGS__), __builtin_FILE(), __builtin_FUNCSIG(), __builtin_LINE())
#else
    #define PW_TRACE(...)     LOGGER_TRACE((__VA_ARGS__), __FILE__, __PRETTY_FUNCTION__, __LINE__)
    #define PW_DEBUG(...)     LOGGER_DEBUG((__VA_ARGS__), __FILE__, __PRETTY_FUNCTION__, __LINE__)
    #define PW_INFO(...)      LOGGER_INFO((__VA_ARGS__), __FILE__, __PRETTY_FUNCTION__, __LINE__)
    #define PW_WARN(...)      LOGGER_WARN((__VA_ARGS__), __FILE__, __PRETTY_FUNCTION__, __LINE__)
    #define PW_ERROR(...)     LOGGER_ERROR((__VA_ARGS__), __FILE__, __PRETTY_FUNCTION__, __LINE__)
    #define PW_CRITICAL(...)  LOGGER_CRITICAL((__VA_ARGS__), __FILE__, __PRETTY_FUNCTION__, __LINE__)
#endif
