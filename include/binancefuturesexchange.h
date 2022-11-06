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

private:
    string BuildTicker24Url();
    string BuildAccountUrl(timestamp_t timestamp);

    bool ParseTicker24(const json::value& value, std::list<Ticker24h>& tickers);
    bool ParseAccount(const json::value& json, AccountInfo &info);
    bool ParseSymbols(const json::value &json, std::list<Symbol> &symbols);

    const string API_PATH = "/fapi";
    const string BINANCE_FUTURES_SERVER = "https://fapi.binance.com";
};

#endif // BINANCEFUTURESEXCHANGE_H
