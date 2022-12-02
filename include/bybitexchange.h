#ifndef BYBITEXCHANGE_H
#define BYBITEXCHANGE_H

#include <exchangeobj.h>

class BybitExchange : public ExchangeObj
{
public:
    BybitExchange();

protected:
    virtual string BuildTimeUrl();
    virtual string BuildSymbolsUrl();
    virtual string BuildTicker24Url();
    virtual string BuildMarketDepthUrl(const string symbol, int limit);
    virtual string BuildAggregateTradesUrl(const string symbol, timestamp_t start_time, timestamp_t end_time, int limit);
    virtual string BuildCandlesUrl(const string symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit);
    virtual string BuildAccountUrl(timestamp_t timestamp);
    virtual string BuildOpenOrdersUrl(timestamp_t timestamp);

    virtual timestamp_t ParseServerTime(const json::value& value);
    virtual bool ParseSymbols(const json::value& value, std::list<Symbol> &symbols);
    virtual bool ParseTicker24(const json::value& value, std::list<Ticker24h>& tickers);
    virtual bool ParseMarketDepth(const json::value& value, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId);
    virtual bool ParseAggregateTradesList(const json::value& value, TradesList& trades);
    virtual bool ParseCandles(const json::value& value, CandlesList& candles);
    virtual bool ParseAccount(const json::value& value, AccountInfo& info);
    virtual bool ParseOpenOrders(const json::value& json, OrderList& orders);

    const string GetInterval(TimeFrame tf);
    string ApiServer_;

private:
    const string BYBIT_SERVER = "https://api.bytick.com";
    const string API_PATH = "";
};

#endif // BYBITEXCHANGE_H
