﻿#ifndef LIBCRYPTOMARKET_H
#define LIBCRYPTOMARKET_H

#include <list>
#include <string>

using std::string;

namespace libcryptomarket {

typedef unsigned long long timestamp_t;

class Symbol
{
public:
    Symbol(bool valid);
    struct Asset {
        string Name ;
        int AssetPrecision;
        int ComissionPrecision;
    };

    const string toString() const   { return Base.Name + Quote.Name; }
    operator string() const         { return Base.Name + Quote.Name; }
    operator const string() const   { return Base.Name + Quote.Name; }

    bool IsValid();
    double GetPriceStep();
    double GetQtyStep();

    void SetPriceStep(double step);
    void SetQtyStep(double step);

    int GetPricePrecision();
    int GetQtyPrecision();

    int QuotePrecison;
    Asset Base;
    Asset Quote;

private:
    bool Valid_;
    double PriceStep_;
    double QtyStep_;
};

struct Price {
    std::string symbol;
    double price;
};

struct Depth {
    double Price;
    double Qty;
};

struct MarketDepthSeries {
    timestamp_t UpdateTime;
    std::list<Depth> Items;
    bool IsBids;
};

enum TimeFrame {
    TimeFrame_1m = 0,
    TimeFrame_5m = 1,
    TimeFrame_15m = 2,
    TimeFrame_1h = 3,
    TimeFrame_4h = 4,
    TimeFrame_1d = 5,
};

struct ExchangeInfo {
    std::list<Symbol> Symbols;
};

struct Balance {
    std::string Asset;
    double Free;
    double Locked;
};

typedef std::list<Balance> Balances;
struct AccountInfo {
    std::string AccountType;
    Balances Balance;
};

struct Trade {
    timestamp_t Time;
    uint64_t Id;
    double Price;
    double Qty;
    bool IsBuy;
};

struct Candle {
    timestamp_t OpenTime;
    timestamp_t CloseTime;
    double Qty;
    double Open;
    double Close;
    double High;
    double Low;
    int NumberOfTrades;
};

typedef std::list<Price> Prices;
typedef std::list<Depth> MarketDepth;
typedef std::list<Trade> TradesList;
typedef std::list<Candle> CandlesList;

struct CryptoMarketHandle {
    void* ExchangeObj;
    string ExchangeName;
};

CryptoMarketHandle NewExchangeObj(const string& name, const std::string &api = "", const std::string &secret = "");
timestamp_t GetServerTime(CryptoMarketHandle& h);

bool GetExchangeInfo(CryptoMarketHandle &h, ExchangeInfo& info);
bool GetAllPrices(CryptoMarketHandle &h, Prices& prices);
bool GetMarketDepth(CryptoMarketHandle &h, const string &symbol, int limit, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId);
bool GetTrades(CryptoMarketHandle &h, const string& symbol, timestamp_t start_time, timestamp_t end_time, int limit, TradesList& trades);
bool GetCandles(CryptoMarketHandle &h, const string& symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit, CandlesList& candles);
bool GetAccount(CryptoMarketHandle &h, AccountInfo& info);

bool Free(CryptoMarketHandle handle);
void Cleanup();

// Web Socket API

static const int MARKET_DEPTH_SUBSCRIBE = 0x1;
static const int TRADES_SUBSCRIBE = 0x2;
static const int CANDLES_SUBSCRIBE_1m = 0x4;
static const int CANDLES_SUBSCRIBE_5m = 0x8;
static const int CANDLES_SUBSCRIBE_15m = 0x10;
static const int CANDLES_SUBSCRIBE_1h = 0x20;
static const int CANDLES_SUBSCRIBE_4h = 0x40;
static const int CANDLES_SUBSCRIBE_1d = 0x80;

typedef void* WebSocketObj;
typedef void (*UpdateMarketDepthEvent)(void*, const std::string&, const std::string&, MarketDepthSeries& series);
typedef void (*AddTradeEvent)(void*, const std::string&, const std::string&, Trade& trade);
typedef void (*UpdateCandleEvent)(void*, const std::string&, const std::string&, TimeFrame tf, Candle& candle);


WebSocketObj CreateWebSocketObj(const std::string& exchange, const std::string& symbol, int subscribe_flags);
bool SetWebSocketContext(WebSocketObj ws, void *context);
void StartWebSocket(WebSocketObj ws);
void StopWebSocket(WebSocketObj ws);
bool SetWebSocketUpdateMarketDepthCallback(WebSocketObj ws, UpdateMarketDepthEvent event);
bool SetWebSocketAddTradeCallback(WebSocketObj ws, AddTradeEvent event);
bool SetWebSocketUpdateCandleCallback(WebSocketObj ws, UpdateCandleEvent);
bool DeleteWebSocket(WebSocketObj ws);

};
#endif // LIBCRYPTOMARKET_H