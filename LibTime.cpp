
#include <string.h>
#include <stdio.h>

#include "LibTime.h"

static struct timeval g_time = {0,0};
static struct timezone g_zone = {0,0};

time_t ReadSysSecond()
{
    g_time.tv_sec = time(NULL);
    return g_time.tv_sec;
}

const struct timeval& ReadSysTime()
{
    gettimeofday(&g_time, &g_zone);
    return g_time;
}

time_t GetSysSecond()
{
    return g_time.tv_sec;
}

const struct timeval& GetSysTime()
{
    return g_time;
}

double GetSysTimeAsDouble()
{
    return TimeToDouble(g_time);
}

uint64_t GetSysTimeAsMilliSecond()
{
    return TimeToMilliSecond(g_time);
}

double GetCurrTimeAsDouble(void)
{
    struct timeval time;

    if (gettimeofday(&time, NULL))
    {
        return 0;
    }

    return TimeToDouble(time);  
}

uint64_t GetCurrTimeAsLong(void)
{
    struct timeval time;

    if (gettimeofday(&time, NULL))
    {
        return 0;
    }

    return TimeToLong(time); 
}

uint64_t GetCurrTimeAsMilliSecond(void)
{
    struct timeval time;

    if (gettimeofday(&time, NULL))
    {
        return 0;
    }

    return TimeToMilliSecond(time);  
}

double TimeToDouble(const struct timeval& time)
{
    return (time.tv_sec + (double)time.tv_usec / MICRO_SECOND);    
}

uint64_t TimeToLong(const struct timeval& time)
{
    return (((uint64_t)time.tv_sec) * MILLI_SECOND + ((uint64_t)time.tv_usec) / MILLI_SECOND);
}

uint64_t TimeToMilliSecond(const struct timeval& time)
{
    return (((uint64_t)time.tv_sec) * MILLI_SECOND  + ((uint64_t)time.tv_usec) / MILLI_SECOND);    
}

std::string TimeToDateString(time_t tt)
{
    struct tm *times = localtime(&tt);
    char buf[80];
    
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d", 
             times->tm_year + 1900, 
             times->tm_mon + 1, 
             times->tm_mday, 
             times->tm_hour, 
             times->tm_min, 
             times->tm_sec);
    
    return buf;
}

std::string MilliTimeToDateString(uint64_t tt)
{
    time_t sec = tt / MILLI_SECOND;
    time_t msec = tt - (sec * MILLI_SECOND);
    struct tm *times = localtime(&sec);
    char buf[80];

    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%.3lu", 
        times->tm_year + 1900, 
        times->tm_mon + 1, 
        times->tm_mday, 
        times->tm_hour, 
        times->tm_min, 
        times->tm_sec,
        msec);

    return buf;
}

std::string MilliTimeToDateString2(uint64_t tt)
{
    time_t sec = tt / MILLI_SECOND;
    time_t msec = tt - (sec * MILLI_SECOND);
    struct tm *times = localtime(&sec);
    char buf[80];

    snprintf(buf, sizeof(buf), "%04d%02d%02d%02d%02d%02d%.3lu", 
        times->tm_year + 1900, 
        times->tm_mon + 1, 
        times->tm_mday, 
        times->tm_hour, 
        times->tm_min, 
        times->tm_sec,
        msec);

    return buf;
}

time_t DateStringToTime(const std::string& s)
{
    struct tm tmTime;
    memset(&tmTime, 0, sizeof(tmTime));

    sscanf(s.data(), "%04d-%02d-%02d %02d:%02d:%02d", 
           &tmTime.tm_year, 
           &tmTime.tm_mon, 
           &tmTime.tm_mday, 
           &tmTime.tm_hour, 
           &tmTime.tm_min, 
           &tmTime.tm_sec);

    tmTime.tm_year -= 1900;
    tmTime.tm_mon -= 1;

    time_t time = mktime(&tmTime);

    return time;
}

time_t DateStringToMilliTime(const std::string& s)
{
    time_t msec = 0;
    struct tm tmTime;
    memset(&tmTime, 0, sizeof(tmTime));

    sscanf(s.data(), "%04d-%02d-%02d %02d:%02d:%02d.%3lu", 
           &tmTime.tm_year, 
           &tmTime.tm_mon, 
           &tmTime.tm_mday, 
           &tmTime.tm_hour, 
           &tmTime.tm_min, 
           &tmTime.tm_sec,
           &msec);

    tmTime.tm_year -= 1900;
    tmTime.tm_mon -= 1;

    time_t time = mktime(&tmTime);

    time = time*MILLI_SECOND + msec;
    return time;
}

time_t DateToTime(int year, int mon, int day, int hour, int min, int sec)
{
    struct tm tmTime;
    memset(&tmTime, 0, sizeof(tmTime));
    
    tmTime.tm_year = year - 1900;
    tmTime.tm_mon = mon - 1;
    tmTime.tm_mday = day;
    tmTime.tm_hour = hour;
    tmTime.tm_min = min;
    tmTime.tm_sec = sec;

    time_t time = mktime(&tmTime);

    return time;
}

std::string GetLastTimeString(double t)
{
    size_t minSeconds = 60;
    size_t hourSeconds = minSeconds * 60;
    size_t daySeconds = hourSeconds * 24;

    size_t last = (size_t)(t>0?t:(-t));

    int days = last / daySeconds;
    last -= days * daySeconds;

    int hours = last / hourSeconds;
    last -= hours * hourSeconds;

    int mins = last / minSeconds;
    last -= mins * minSeconds;

    int secs = last;

    char buf[80];
    snprintf(buf, sizeof(buf)-1, "%d days, %02d:%02d:%02d", days, hours, mins, secs);

    return buf;
}

void SleepInDouble(double seconds)
{
    struct timespec req;
    struct timespec rem;

    req.tv_sec = (long long)seconds;
    req.tv_nsec = (long long)(((long long)(seconds - req.tv_sec)) * NANO_SECOND);

    while (nanosleep(&req, &rem))
    {
        req = rem;
    }
}


uint64_t GetSecondSn()
{
    static uint32_t next = 0;
    uint64_t sn = GetSysSecond();
    sn = (sn<<32)+(++next);
    
    return sn;
}

