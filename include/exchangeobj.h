#ifndef EXCHANGEOBJ_H
#define EXCHANGEOBJ_H

#include "curl.h"

#include <boost/json.hpp>
#include <list>
#include <string>
#include <vector>
#include <map>

#include "libcryptomarket.h"
#include "loggerobj.h"

using std::list;
using std::string;
using std::map;
using std::vector;
using namespace libcryptomarket;
namespace json = boost::json;

class LogObject
{
public:
    LogObject() {}
};

class ExchangeObj
{
public:
    ExchangeObj();
    virtual ~ExchangeObj();

    void Init(string api, string secret);
    void Cleanup();

    timestamp_t GetServerTime();
    bool GetExchangeInfo(ExchangeInfo& info);
    bool GetAllPrices(Prices& prices);
    bool GetMarketDepth(const string &symbol, int limit, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId);
    bool GetTrades(const string& symbol, timestamp_t start_time, timestamp_t end_time, int limit, TradesList& trades);
    bool GetCandles(const string& symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit, CandlesList& candles);
    bool GetAccount(AccountInfo& info);

    void SetLogger(LoggerObj* logger) { Logger_ = logger; }

protected:
    string ApiKey_;
    string SecretKey_;
    CURL* curl;
    int RecvWindow_;

    LoggerObj* Logger_;
    void GetUrl(string &url, string &result_json);
    void GetUrlWithHeader(string &url, string &str_result, vector<string> &extra_http_header, string &post_data, string &action);

    virtual string BuildTimeUrl() = 0;
    virtual string BuildExchangeInfoUrl() = 0;
    virtual string BuildAllPricesUrl() = 0;
    virtual string BuildMarketDepthUrl(const string symbol, int limit) = 0;
    virtual string BuildAggregateTradesUrl(const string symbol, timestamp_t start_time, timestamp_t end_time, int limit) = 0;
    virtual string BuildCandlesUrl(const string symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit) = 0;
    virtual string BuildAccountUrl(timestamp_t timestamp) = 0;

    virtual timestamp_t ParseServerTime(const json::value& value) = 0;
    virtual bool ParseExchangeInfo(const json::value& value, ExchangeInfo& info) = 0;
    virtual bool ParseAllPrices(const json::value& value, Prices& prices) = 0;
    virtual bool ParseMarketDepth(const json::value& value, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId) = 0;
    virtual bool ParseAggregateTradesList(const json::value& value, TradesList& trades) = 0;
    virtual bool ParseCandles(const json::value& value, CandlesList& candles) = 0;
    virtual bool ParseAccount(const json::value& value, AccountInfo& info) = 0;

    virtual string Timeframe2String(TimeFrame tf);
    virtual bool IsError(const json::value &result);

    void Log(const char* msg, ...);

    typedef size_t (*Callback)(char *content, size_t size, size_t nmemb, string *buffer);
    Callback CUrlCallback_;
};

#endif // EXCHANGEOBJ_H
