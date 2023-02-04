#ifndef EXCHANGEOBJ_H
#define EXCHANGEOBJ_H

#include <boost/json.hpp>
#include <list>
#include <string>
#include <vector>
#include <map>

#include "libcryptomarket.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/json.hpp>

using std::list;
using std::string;
using std::map;
using std::vector;
using namespace libcryptomarket;

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
    bool GetTrades(const std::string &symbol, timestamp_t start_time, timestamp_t end_time, int limit, TradesList& trades);
    bool GetCandles(const string& symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit, CandlesList& candles);

    bool GetAccount(AccountInfo &info);
    bool GetOpenOrders(OrderList& orders);
    bool NewOrder(OrderType type, std::string& symbol, Direct direct, double qty, double price, double stopPrice, Order& newOrder);
    bool CancelOrder(Order& order);
    int GetErrorCode()              { return ErrorCode_;    }
    std::string GetErrorMessage()   { return ErrorMessage_; }

    void SetLogger(BaseLogger* logger) { Logger_ = logger; }

protected:
    std::string HOST;
    std::string PORT;
    string ApiKey_;
    string SecretKey_;
    int RecvWindow_;
    std::string ErrorMessage_;
    int ErrorCode_;

    virtual const string GetTimeEndpoint() = 0;
    virtual timestamp_t ParseServerTime(const boost::json::value& value) = 0;

    virtual const string GetSymbolsEndpoint() = 0;
    virtual bool ParseSymbols(const boost::json::value& value, std::list<Symbol> &symbols) = 0;

    virtual const string GetAccountInfoEndpoint(timestamp_t time) = 0;
    virtual bool ParseAccountInfo(const boost::json::value& value, AccountInfo& info) = 0;

    virtual const string GetTicker24Endpoint() = 0;
    virtual bool ParseTicker24(boost::json::value& value, std::list<Ticker24h>& tickers) = 0;

    virtual const string GetMarketDepthEndpoint(const std::string& symbol, int limit) = 0;
    virtual bool ParseMarketDepth(const boost::json::value &json, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId) = 0;

    virtual const string GetTradesEndpoint(const std::string& symbol, timestamp_t start_time, timestamp_t end_time, int limit) = 0;
    virtual bool ParseTrades(boost::json::value& value, TradesList& trades) = 0;

    virtual const string GetCandlesEndpoint(const std::string& symbol, TimeFrame tf, timestamp_t start, timestamp_t end, int limit) = 0;
    virtual bool ParseCandles(boost::json::value& value, CandlesList& candles) = 0;

    virtual const string GetOpenOrdersEndpoint(timestamp_t time) = 0;
    virtual bool ParseOrders(boost::json::value& value, OrderList& orders) = 0;

    virtual const string GetNewOrderEndpoint(timestamp_t time, const string& symbol, Direct direct, OrderType type, double qty, double price, double stopPrice) = 0;
    virtual bool ParseNewOrder(boost::json::value& value, Order& newOrder) = 0;

    virtual const string GetCancelOrderEndpoint(timestamp_t time, Order& order) = 0;
    virtual bool ParseCancelOrder(boost::json::value& value) = 0;

    string hmac_sha256(const char *key, const char *data);
    string b2a_hex(char *byte_arr, int n);

    BaseLogger* Logger_;

    virtual string TimeFrame2String(TimeFrame tf);
    virtual bool IsError(const boost::json::value &result);
    void ErrorMessage(const std::string &msg);
    void Log(BaseLogger::Level lv, const string& msg);
    void InfoMessage(const std::string &msg);
    void WarningMessage(const std::string &msg);

    void GetUrl(const std::string &path, string &result_json);
    void PutUrl(const std::string &path, string &result_json);
    void PostUrl(const std::string &path, string &result_json);
    void DeleteUrl(const std::string &path, string &result_json);

private:
    void GetUrlWithHeader(const std::string &path, boost::beast::http::verb action, string &str_result);
};

#endif // EXCHANGEOBJ_H
