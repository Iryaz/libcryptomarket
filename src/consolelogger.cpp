#include "consolelogger.h"
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include "sys/time.h"

ConsoleLogger::ConsoleLogger()
{

}

void ConsoleLogger::Log(const char* msg, ...)
{
    if (Enable_ == false)
        return;

    char new_msg[1024];
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    time_t t = tv.tv_sec;
    struct tm * now = localtime(&t);
    sprintf(new_msg, "%04d-%02d-%02d %02d:%02d:%02d %06ld :%s\n", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, tv.tv_usec, msg);

    va_list arg;
    va_start(arg, msg);

    vfprintf(stdout, new_msg, arg);
    fflush(stdout);

    va_end (arg);
}
