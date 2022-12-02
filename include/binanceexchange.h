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

    Direct String2OrderSide(std::string s);
    OrderStatus String2OrderStatus(std::string s);
    OrderType String2OrderType(std::string s);

private:
    virtual timestamp_t ParseServerTime(const json::value& json);
    virtual bool ParseSymbols(const json::value& json, std::list<Symbol> &symbols);
    virtual bool ParseTicker24(const json::value& value, std::list<Ticker24h>& tickers);
    virtual bool ParseMarketDepth(const json::value& json, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId);
    virtual bool ParseAggregateTradesList(const json::value& json, TradesList& trades);
    virtual bool ParseCandles(const json::value& json, CandlesList& candles);
    virtual bool ParseAccount(const json::value& json, AccountInfo& info);
    virtual bool ParseOpenOrders(const json::value& json, OrderList& orders);

    virtual string BuildTimeUrl();
    virtual string BuildSymbolsUrl();
    virtual string BuildTicker24Url();
    virtual string BuildMarketDepthUrl(const string symbol, int limit);
    virtual string BuildAggregateTradesUrl(const string symbol, timestamp_t start_time, timestamp_t end_time, int limit);
    virtual string BuildCandlesUrl(const string symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit);
    virtual string BuildAccountUrl(timestamp_t timestamp);
    virtual string BuildOpenOrdersUrl(timestamp_t timestamp);

    const string BINANCE_SERVER = "https://api.binance.com";
    const string API_PATH = "/api";
};

#endif // BINANCEEXCHANGE_H
