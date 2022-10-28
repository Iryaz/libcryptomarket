#include <iostream>
#include <libcryptomarket.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include "sys/time.h"

using namespace libcryptomarket;

void ConsoleLogger::Log(Level lv, const string& msg)
{
    char new_msg[1024];
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    time_t t = tv.tv_sec;
    struct tm * now = localtime(&t);
    sprintf(new_msg, "%04d-%02d-%02d %02d:%02d:%02d %06ld: %s\n", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, tv.tv_usec, msg.c_str());
    switch (lv) {
    case Info:
        std::cout << new_msg << "\n";
        break;
    case Critical:
    case Warning:
        std::cerr << new_msg << "\n";
        break;
    default:
        break;
    }
}
