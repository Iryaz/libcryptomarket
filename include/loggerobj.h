#ifndef LOGGEROBJ_H
#define LOGGEROBJ_H


class LoggerObj
{
public:
    LoggerObj();

    void SetEnable(bool en) { Enable_ = en; }
    virtual void Log(const char *message, ...) = 0;

protected:
    bool Enable_;
};

#endif // LOGGEROBJ_H
