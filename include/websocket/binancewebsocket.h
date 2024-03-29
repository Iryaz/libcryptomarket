﻿#ifndef BINANCEWEBSOCKET_H
#define BINANCEWEBSOCKET_H

#include "exchangeobj.h"
#include "websocket/basewebsocket.h"

class BinanceWebSocket : public BaseWebSocket
{
public:
    BinanceWebSocket(Type type, const std::string& symbol, int subscribe_flags, const std::string &listen_key);
    ~BinanceWebSocket();

protected:
    void SetSymbol(const std::string& symbol);
    void Init(int flag, const std::string &listen_key);
    void ParseMarketDepth(const json::value& json);
    void ParseTrades(const json::value& json);
    void ParseKLines(const json::value& json);
    void ParseMarginCall(const json::value& json);
    void ParseAccountUpdate(const json::value& json);
    void ParseOrderTrade(const json::value& json);
    void ParseMarkPrice(const json::value& json);
    virtual bool StartLoop();

private:
    bool IsNull(boost::json::value& val);
    virtual void ParseJSon(boost::json::value& result);
    void ParseDataSpot(boost::json::value& result);
    void ParseDataFutures(boost::json::value& result);
    DataEventType String2EventType(const std::string& s);
    Direct String2Direct(const std::string& s);
    OrderStatus String2OrderStatus(std::string s);
    OrderType String2OrderType(std::string s);
    MarginType String2MarginType(const std::string& type);
    std::string PathParams_;
    ExchangeObj *BinanceObj_;
    uint64_t LastUpdateId_;
};

#endif // BINANCEWEBSOCKET_H
