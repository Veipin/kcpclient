
#include <ctype.h>

#include "LibLog.h"

#ifdef LOGGROUP
bool g_isLogGroup = false;
void SetAppLogLogGroup(bool isOpen)
{
    g_isLogGroup = isOpen;
}
#endif

bool g_isLogPrint = false;

void SetAppLogPrint(bool isOpen)
{
    g_isLogPrint = isOpen;
}

bool g_isLogAndroid = false;

void SetAppLogAndroid(bool isOpen)
{
    g_isLogAndroid = isOpen;
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
void PrintError(const char* file, const int line, const char* format, ... )
{
    char buf[1024] = "";
    char* out = buf;
    int size = sizeof(buf)-1;
    
    int rc = snprintf(out, size, "%s:%d ", file, line);
    size -= rc;
    out += rc;
    
    va_list ap;
    va_start(ap, format);
    vsnprintf(out, size, format, ap);
    va_end(ap);

    ::perror(buf);
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
std::ostream& DumpData(std::ostream& out, 
                       const void* data, 
                       size_t size, 
                       const void* printAddr,
                       bool hasAddr, 
                       bool hasHex, 
                       bool hasChar)
{
    const char* start = (const char*)data;
    const char* end = (const char*)data + size;
    off_t offset = (off_t)printAddr;

    const char* p = start;
    
    while (p < end)
    {
        std::ostringstream addrStr;
        std::ostringstream hexStr;
        std::ostringstream charStr;
        char line[80];

        sprintf(line, "%08lX:", p - start + offset);
        addrStr << line;

        int no = 0;
        while (no < 16)
        {
            if (p >= end)
            {
                hexStr << "   ";
                charStr << " ";
            }
            else
            {
                unsigned char ch = *p;
                
                sprintf(line, "%02X ", ch);
                hexStr << line;

                if (isprint(ch))
                {
                    charStr << ch;
                }
                else
                {
                    charStr << ".";
                }
            }

            if (7 == no)
            {
                hexStr << " ";
            }

            ++no;
            ++p;
        }

        if (hasAddr)
        {
            out << addrStr.str().data() << "  ";
        }

        if (hasHex)
        {
            out << hexStr.str().data() << "  ";
        }
        
        if (hasChar)
        {
            out << charStr.str().data() << std::endl;
        }
    }

    return out;    
}

