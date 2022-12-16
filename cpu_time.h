/**
 * @file cpu_time.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-12-14
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */
#ifndef CPU_TIME_H
#define CPU_TIME_H
#include <stdint.h>
#include <stdbool.h>

#if defined(_WIN32) || defined(_WIN64)
// define something for Windows (64-bit only)
#include <windows.h>
#include <time.h>
uint64_t getCurrentSecTimestamp();
uint64_t getCurrentMilliSecTimestamp();
#elif __APPLE__
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#elif TARGET_OS_IPHONE
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#endif
#elif __ANDROID__
// android
#elif __linux__
// linux
#include <sys/time.h>
#include <unistd.h>

#elif __unix__ // all unices not caught above
// Unix
#elif defined(_POSIX_VERSION)
// POSIX
#else
#error "Unknown"
#endif

uint64_t getCurrentSecTimestamp();
uint64_t getCurrentMilliSecTimestamp();
void delayMs(uint64_t ms);
bool period_query_user(uint64_t *period_recorder, uint64_t period);
#endif //! CPU_TIME_H