#pragma once

extern "C" {
    __declspec(dllimport) void LOGGER_TRACE(const char* message, const char* file, const char* func, int line);
    __declspec(dllimport) void LOGGER_DEBUG(const char* message, const char* file, const char* func, int line);
    __declspec(dllimport) void LOGGER_INFO(const char* message, const char* file, const char* func, int line);
    __declspec(dllimport) void LOGGER_WARN(const char* message, const char* file, const char* func, int line);
    __declspec(dllimport) void LOGGER_ERROR(const char* message, const char* file, const char* func, int line);
    __declspec(dllimport) void LOGGER_CRITICAL(const char* message, const char* file, const char* func, int line);
}