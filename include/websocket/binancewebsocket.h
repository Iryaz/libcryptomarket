#ifndef BINANCEWEBSOCKET_H
#define BINANCEWEBSOCKET_H

#include "exchangeobj.h"
#include "websocket/basewebsocket.h"

class BinanceWebSocket : public BaseWebSocket
{
public:
    BinanceWebSocket(Type type, const std::string& symbol, int subscribe_flags);
    ~BinanceWebSocket();

protected:
    void SetSymbol(const std::string& symbol);
    void Init(int flag);
    void ParseMarketDepth(const json::value& json);
    void ParseTrades(const json::value& json);
    void ParseKLines(const json::value& json);
    virtual bool StartLoop();

private:
    virtual void ParseJSon(boost::json::value& result);
    void ParseDataSpot(boost::json::value& result);
    void ParseDataFutures(boost::json::value& result);
    DataEventType String2EventType(const std::string& s);
    std::string PathParams_;
    ExchangeObj *BinanceObj_;
    uint64_t LastUpdateId_;
};

#endif // BINANCEWEBSOCKET_H
