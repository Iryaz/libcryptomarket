#ifndef ByBitWebsocket_H
#define ByBitWebsocket_H

#include "websocket/basewebsocket.h"

class ByBitWebsocket : public BaseWebSocket
{
public:
    ByBitWebsocket(Type type, const std::string& symbol, int subscribe_flags);

protected:
    void Init(int flag, const std::string& listen_key = "");
    virtual void SetSymbol(const std::string& symbol);

    void ParseMarketDepth(const json::value& json);
    void ParseTrades(const json::value& json);
    void ParseKLines(const json::value& json);

private:
    void ParseJSon(boost::json::value& result);
    DataEventType String2EventType(const std::string& s);
    bool IsResponseMessage(const json::value& result);
    TimeFrame GetTimeFrameFromPeriod(int period);
};

#endif // ByBitWebsocket_H
