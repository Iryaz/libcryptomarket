#ifndef EXCHANGEOBJ_H
#define EXCHANGEOBJ_H

#include "curl.h"

#include <boost/json.hpp>
#include <list>
#include <string>
#include <vector>
#include <map>

#include "libcryptomarket.h"

using std::list;
using std::string;
using std::map;
using std::vector;
using namespace libcryptomarket;
namespace json = boost::json;

class ExchangeObj
{
public:
    ExchangeObj();
    virtual ~ExchangeObj();

    void Init(string api, string secret);
    void Cleanup();

    timestamp_t GetServerTime();
    bool GetSymbols(std::list<Symbol> &symbols);
    bool GetTicker24(std::list<Ticker24h>& tickers);
    bool GetMarketDepth(const string &symbol, int limit, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId);
    bool GetTrades(const string& symbol, timestamp_t start_time, timestamp_t end_time, int limit, TradesList& trades);
    bool GetCandles(const string& symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit, CandlesList& candles);
    bool GetAccount(AccountInfo& info);
    bool GetOpenOrders(OrderList& orders);
    bool NewOrder(OrderType type, std::string& symbol, Direct direct, double qty, double price, Order& newOrder);
    bool CancelOrder(Order &order);

    void SetLogger(BaseLogger* logger) { Logger_ = logger; }

protected:
    string ApiKey_;
    string SecretKey_;
    CURL* curl;
    int RecvWindow_;

    string hmac_sha256(const char *key, const char *data);
    string b2a_hex(char *byte_arr, int n);

    BaseLogger* Logger_;
    void GetUrl(string &url, string &result_json);
    void GetUrlWithHeader(string &url, string &str_result, vector<string> &extra_http_header, string &post_data, string &action);

    virtual string BuildTimeUrl() = 0;
    virtual string BuildSymbolsUrl() = 0;
    virtual string BuildTicker24Url() = 0;
    virtual string BuildMarketDepthUrl(const string symbol, int limit) = 0;
    virtual string BuildAggregateTradesUrl(const string symbol, timestamp_t start_time, timestamp_t end_time, int limit) = 0;
    virtual string BuildCandlesUrl(const string symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit) = 0;
    virtual string BuildAccountUrl(timestamp_t timestamp) = 0;
    virtual string BuildOpenOrdersUrl(timestamp_t timestamp) = 0;
    virtual string BuildNewOrderUrl(timestamp_t timestamp, const std::string &symbol, OrderType type, Direct direct, double qty, double price) = 0;
    virtual string BuildCancelOrderUrl(timestamp_t timestamp, Order &order) = 0;

    virtual timestamp_t ParseServerTime(const json::value& value) = 0;
    virtual bool ParseSymbols(const json::value& value, std::list<Symbol> &symbols) = 0;
    virtual bool ParseTicker24(const json::value& value, std::list<Ticker24h>& tickers) = 0;
    virtual bool ParseMarketDepth(const json::value& value, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId) = 0;
    virtual bool ParseAggregateTradesList(const json::value& value, TradesList& trades) = 0;
    virtual bool ParseCandles(const json::value& value, CandlesList& candles) = 0;
    virtual bool ParseAccount(const json::value& value, AccountInfo& info) = 0;
    virtual bool ParseOpenOrders(const json::value& value, OrderList& orders) = 0;
    virtual bool ParseNewOrder(const json::value& value, Order& order) = 0;
    virtual bool ParseCancelOrder(const json::value& value) = 0;

    virtual string Timeframe2String(TimeFrame tf);
    virtual bool IsError(const json::value &result);

    void Log(BaseLogger::Level lv, const string& msg);
    void ErrorMessage(const std::string &msg);
    void InfoMessage(const std::string &msg);
    void WarningMessage(const std::string &msg);

    typedef size_t (*Callback)(char *content, size_t size, size_t nmemb, string *buffer);
    Callback CUrlCallback_;
};

#endif // EXCHANGEOBJ_H
