#ifndef BYBITFUTURESEXCHANGE_H
#define BYBITFUTURESEXCHANGE_H

#include "bybitexchange.h"

class BybitFuturesExchange : public BybitExchange
{
public:
    BybitFuturesExchange();

private:
    virtual string BuildTimeUrl();
    virtual string BuildSymbolsUrl();
    virtual string BuildTicker24Url();
    virtual string BuildMarketDepthUrl(const string symbol, int limit);
    virtual string BuildAggregateTradesUrl(const string symbol, timestamp_t start_time, timestamp_t end_time, int limit);
    virtual string BuildCandlesUrl(const string symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit);
    virtual string BuildAccountUrl(timestamp_t timestamp);
    virtual string GetListenKeyUrl();

    virtual bool ParseTicker24(const json::value& value, std::list<Ticker24h>& tickers);
    virtual timestamp_t ParseServerTime(const json::value& value);
    bool ParseSymbols(const json::value& value, std::list<Symbol> &symbols);
};

#endif // BYBITFUTURESEXCHANGE_H
