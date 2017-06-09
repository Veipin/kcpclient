#ifndef __LIB_LOG_H__
#define __LIB_LOG_H__

#include <iostream>
#include <sstream>
#include <stdarg.h>

#ifdef USE_NDK_ENV
#include <android/log.h>
#else
#define __android_log_print(type, tag, format, args...)
#endif

#ifdef LOGGROUP
    #include "log.h"
    extern bool g_isLogGroup;
    extern void SetAppLogLogGroup(bool isOpen);
#else
    #define g_isLogGroup true
    #define LogGroup(group, format, args...) __android_log_print(ANDROID_LOG_DEBUG, "libsimterm.so", format, ##args); 
#endif

extern bool g_isLogPrint;

extern bool g_isLogAndroid;
extern void SetAppLogAndroid(bool isOpen);

#define AppLog(group, format, args...)\
{\
    if (g_isLogGroup) {LogGroup(group, format, ##args);}\
    if (g_isLogPrint) printf(format, ##args);\
    if (g_isLogAndroid) {__android_log_print(ANDROID_LOG_DEBUG, "libsimterm.so", format, ##args);}\
}
    
extern void SetAppLogPrint(bool isOpen);

/// base name of current file
#define BASENAME_OF_FILE (strrchr(__FILE__,'/') != 0 ? strrchr(__FILE__,'/')+1 : __FILE__)

////////////////////////////////////////////////////////////////////////////////
/// @brief Print a message and error string
/// @param[in] file - file name
/// @param[in] line - file line
/// @param[in] format - output format
/// @return none
////////////////////////////////////////////////////////////////////////////////
extern void PrintError(const char* file, const int line, const char* format, ... );

/// define name of pError
#define PERROR(format, args...) PrintError(BASENAME_OF_FILE, __LINE__, format, ##args)

////////////////////////////////////////////////////////////////////////////////
/// @brief Print data
/// @param[out] out - output string
/// @param[in] data - data buffer
/// @param[in] size - data size
/// @param[in] printAddr - the begin address to print
/// @param[in] hasAddr - if print address
/// @param[in] hasHex - if print hex data
/// @param[in] hasChar - if print char data
/// @return a reference to output string
////////////////////////////////////////////////////////////////////////////////
extern std::ostream& DumpData(std::ostream& out, 
                              const void* data, 
                              size_t size, 
                              const void* printAddr = 0,
                              bool hasAddr = true, 
                              bool hasHex = true, 
                              bool hasChar = true);


#endif // __LIB_LOG_H__

