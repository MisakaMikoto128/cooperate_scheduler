/**
 * @file cpu_time.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-14
 * 
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 * 
 */
#include "cpu_time.h"
#if defined(_WIN32) || defined(_WIN64)
//define something for Windows (64-bit only)
#include <windows.h>
#include <time.h>
uint64_t getCurrentSecTimestamp(){
    time_t t;
    time(&t);
    return t;
}

uint64_t getCurrentMilliSecTimestamp(){
    FILETIME file_time;
    GetSystemTimeAsFileTime(&file_time);
    uint64_t time = ((uint64_t)file_time.dwLowDateTime) + ((uint64_t)file_time.dwHighDateTime << 32);

    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970
    uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

    return (uint64_t)((time - EPOCH) / 10000LL);
}
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
uint64_t getCurrentSecTimestamp(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

uint64_t  getCurrentMilliSecTimestamp(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

#elif __unix__ // all unices not caught above
// Unix
#elif defined(_POSIX_VERSION)
// POSIX
#else
#error "Unknown"
#endif

void delayMs(uint64_t ms)
{
   uint64_t start = 0;
   start =  getCurrentMilliSecTimestamp();
   while ((getCurrentMilliSecTimestamp() - start) >= ms)
   {
   }
}

/**
 * @brief 同period_query_user，只是时间记录再一个uint32_t*指向的变量中。
 *
 * @param period_recorder 记录运行时间的变量的指针。
 * @param period 周期。
 * @return true 周期到了
 * @return false 周期未到。
 */
bool period_query_user(uint64_t *period_recorder, uint64_t period)
{
    bool ret = false;
    // 这里一定是>=，如果是 > ，那么在1 cpu tick间隔的时候时间上是2cpu tick执行一次。
    // 这里不允许period为0，不然就会失去调度作用。
    if ((getCurrentMilliSecTimestamp() - *period_recorder) >= period)
    {
        *period_recorder = getCurrentMilliSecTimestamp();
        ret = true;
    }
    return ret;
}