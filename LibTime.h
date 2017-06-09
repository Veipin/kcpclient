#ifndef __LIB_TIME_H__
#define __LIB_TIME_H__

#include <string>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>

#define MILLI_SECOND            (1000lu)
#define MICRO_SECOND            (1000000lu)
#define NANO_SECOND             (1000000000lu)


extern time_t ReadSysSecond();
extern const struct timeval& ReadSysTime();

extern time_t GetSysSecond();
extern const struct timeval& GetSysTime();
extern double GetSysTimeAsDouble();
extern double GetCurrTimeAsDouble(void);
extern uint64_t GetCurrTimeAsLong(void);
extern uint64_t GetSysTimeAsMilliSecond();
extern uint64_t GetCurrTimeAsMilliSecond(void);

extern double TimeToDouble(const struct timeval& time);
extern uint64_t TimeToLong(const struct timeval& time);
extern uint64_t TimeToMilliSecond(const struct timeval& time);
extern std::string TimeToDateString(time_t tt);
extern std::string MilliTimeToDateString(uint64_t tt);
extern std::string MilliTimeToDateString2(uint64_t tt);
extern time_t DateStringToTime(const std::string& s);
extern time_t DateStringToMilliTime(const std::string& s);
extern time_t DateToTime(int year, int mon, int day, int hour, int min, int sec);
extern std::string GetLastTimeString(double t);

extern void SleepInDouble(double seconds);

extern uint64_t GetSecondSn();

class UseTime
{
public:
    
    UseTime()
    {
        Restart();
    }
    
    ~UseTime()
    {
    }
    
    double Get()
    {
        struct timeval end;
        gettimeofday(&end, NULL);
        
        double out = (end.tv_sec - m_start.tv_sec) + (double)(end.tv_usec - m_start.tv_usec) / MICRO_SECOND;
        
        return out;
    }

    void Restart()
    {
        gettimeofday(&m_start, NULL);
    }
        
private:
    
    struct timeval m_start;
};

#endif // __LIB_TIME_H__

