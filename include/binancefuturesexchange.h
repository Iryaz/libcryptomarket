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
    bool ParseAccount(const json::value& json, AccountInfo &info);
    string BuildAccountUrl(timestamp_t timestamp);

    const string API_PATH = "/fapi";
    const string BINANCE_FUTURES_SERVER = "https://fapi.binance.com";
};

#endif // BINANCEFUTURESEXCHANGE_H
