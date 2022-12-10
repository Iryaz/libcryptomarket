#ifndef BINANCEFUTURESEXCHANGE_H
#define BINANCEFUTURESEXCHANGE_H

#include "binanceexchange.h"
#include <boost/json.hpp>

namespace json = boost::json;

class BinanceFuturesExchange : public BinanceExchange
{
public:
    BinanceFuturesExchange();
    ~BinanceFuturesExchange();

    bool SetMarginOptions(std::string& symbol, FuturesMarginOption &options);
    bool GetMarginOptions(std::string& symbol, FuturesMarginOption &options);

private:
    string BuildTicker24Url();
    string BuildAccountUrl(timestamp_t timestamp);

    bool ParseMarginOptions(const json::value& value, FuturesMarginOption& options);
    bool ParseTicker24(const json::value& value, std::list<Ticker24h>& tickers);
    bool ParseAccount(const json::value& json, AccountInfo &info);
    bool ParseSymbols(const json::value &json, std::list<Symbol> &symbols);
    bool CheckSetMarginType(const json::value &json);
    bool CheckSetLeverage(const json::value &json);
    string GetListenKeyUrl();

    bool SetMarginType(timestamp_t time, std::string& symbol, MarginType type);
    bool SetLeverage(timestamp_t time, std::string& symbol, int leverage);

    const string API_PATH = "/fapi";
    const string BINANCE_FUTURES_SERVER = "https://fapi.binance.com";
};

#endif // BINANCEFUTURESEXCHANGE_H
