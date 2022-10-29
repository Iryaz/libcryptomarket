#ifndef BINANCEWEBSOCKET_H
#define BINANCEWEBSOCKET_H

#include "websocket/basewebsocket.h"

class BinanceWebSocket : public BaseWebSocket
{
public:
    enum Type {
        Spot,
        Futures
    };

    BinanceWebSocket(Type type, const std::string& symbol, int subscribe_flags);

protected:
    void SetSymbol(const std::string& symbol);
    void Init(int flag);
    void ParseMarketDepth(const json::value& json);
    void ParseTrades(const json::value& json);
    void ParseKLines(const json::value& json);

private:
    virtual void ParseJSon(boost::json::value& result);
    void ParseDataSpot(boost::json::value& result);
    void ParseDataFutures(boost::json::value& result);
    DataEventType String2EventType(const std::string& s);
    std::string PathParams_;
    Type Type_;
};

#endif // BINANCEWEBSOCKET_H
