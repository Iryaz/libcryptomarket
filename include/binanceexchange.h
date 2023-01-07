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

    bool GetListenKey(std::string &listen_key);
    bool PutListenKey(const std::string& key);
    bool CloseListenKey(const std::string& key);

protected:
    const string GetTimeEndpoint();
    virtual timestamp_t ParseServerTime(const json::value& json);

    virtual const std::string GetSymbolsEndpoint();
    virtual bool ParseSymbols(const boost::json::value& json, std::list<Symbol> &symbols);

    virtual const std::string GetListenKeyEndpoint();
    virtual bool ParseListenKey(const json::value& value, std::string& key);

    virtual const string GetAccountInfoEndpoint(timestamp_t time);
    virtual bool ParseAccountInfo(const boost::json::value& json, AccountInfo& info);

    virtual const string GetTicker24Endpoint();
    virtual bool ParseTicker24(json::value &value, std::list<Ticker24h>& tickers);

    virtual const string GetMarketDepthEndpoint(const std::string& symbol, int limit);
    virtual bool ParseMarketDepth(const boost::json::value &json, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId);

    virtual const string GetTradesEndpoint(const std::string& symbol, timestamp_t start_time, timestamp_t end_time, int limit);
    virtual bool ParseTrades(boost::json::value& value, TradesList& trades);

    virtual const string GetCandlesEndpoint(const std::string& symbol, TimeFrame tf, timestamp_t start, timestamp_t end, int limit);
    virtual bool ParseCandles(boost::json::value& value, CandlesList& candles);

    virtual const std::string GetOpenOrdersEndpoint(timestamp_t time);
    virtual bool ParseOrders(boost::json::value& value, OrderList& orders);

    virtual const std::string GetNewOrderEndpoint(timestamp_t time, const string& symbol, Direct direct, OrderType type, double qty, double price, double stopPrice);
    virtual bool ParseNewOrder(boost::json::value& value, Order& order);

    virtual const string GetCancelOrderEndpoint(timestamp_t time, Order& order);
    virtual bool ParseCancelOrder(boost::json::value& value);

    Direct String2OrderSide(std::string s);
    OrderStatus String2OrderStatus(std::string s);
    OrderType String2OrderType(std::string s);
    string OrderSide2String(Direct direct);
    string OrderType2String(OrderType type);

    string API;
};

#endif // BINANCEEXCHANGE_H
