#ifndef BINANCEFUTURESEXCHANGE_H
#define BINANCEFUTURESEXCHANGE_H

#include "binanceexchange.h"
#include <boost/json.hpp>

class BinanceFuturesExchange : public BinanceExchange
{
public:
    BinanceFuturesExchange();
    ~BinanceFuturesExchange();

    bool GetMarginOptions(std::string& symbol, FuturesMarginOption &options);
    bool SetMarginOptions(std::string& symbol, FuturesMarginOption &options);
    bool GetCurrentPosition(const std::string &symbol, std::list<Position>& pos);

protected:
    const string GetTicker24Endpoint();
    const std::string GetListenKeyEndpoint();
    const string GetAccountInfoEndpoint(timestamp_t time);

private:
    bool ParseMarginOptions(const json::value& value, FuturesMarginOption &options);
    bool SetMarginType(timestamp_t time, std::string& symbol, MarginType type);
    bool SetLeverage(timestamp_t time, std::string& symbol, int leverage);
    bool CheckSetMarginType(const json::value &json);
    bool CheckSetLeverage(const json::value &json);
    bool ParseSymbols(const boost::json::value& json, std::list<Symbol> &symbols);
    bool ParseAccountInfo(const boost::json::value& json, AccountInfo& info);

    const string GetPositionEndpoint(timestamp_t time, const string& symbol);
    bool ParseCurrentPosition(boost::json::value& value, std::list<Position>& pos);
};

#endif // BINANCEEXCHANGE_H
