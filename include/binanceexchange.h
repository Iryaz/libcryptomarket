#ifndef BINANCEEXCHANGE_H
#define BINANCEEXCHANGE_H

#include "exchangeobj.h"
#include <boost/json.hpp>

namespace json = boost::json;

class BinanceExchange : public ExchangeObj
{
public:
    BinanceExchange();
    ~BinanceExchange();

protected:
    string ApiType_;
    string ApiServer_;

    string hmac_sha256(const char *key, const char *data);
    string b2a_hex(char *byte_arr, int n);

private:
    virtual timestamp_t ParseServerTime(const json::value& json);
    virtual bool ParseExchangeInfo(const json::value& json, ExchangeInfo& info);
    virtual bool ParseAllPrices(const json::value& json, Prices &prices);
    virtual bool ParseMarketDepth(const json::value& json, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId);
    virtual bool ParseAggregateTradesList(const json::value& json, TradesList& trades);
    virtual bool ParseCandles(const json::value& json, CandlesList& candles);
    virtual bool ParseAccount(const json::value& json, AccountInfo& info);

    virtual string BuildTimeUrl();
    virtual string BuildExchangeInfoUrl();
    virtual string BuildAllPricesUrl();
    virtual string BuildMarketDepthUrl(const string symbol, int limit);
    virtual string BuildAggregateTradesUrl(const string symbol, timestamp_t start_time, timestamp_t end_time, int limit);
    virtual string BuildCandlesUrl(const string symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit);
    virtual string BuildAccountUrl(timestamp_t timestamp);

    const string BINANCE_SERVER = "https://api.binance.com";
    const string API_PATH = "/api";
};

#endif // BINANCEEXCHANGE_H
