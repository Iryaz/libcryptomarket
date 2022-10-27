#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H

#include "loggerobj.h"

class ConsoleLogger : public LoggerObj
{
public:
    ConsoleLogger();
    void Log(const char* msg, ...);
};

#endif // CONSOLELOGGER_H
