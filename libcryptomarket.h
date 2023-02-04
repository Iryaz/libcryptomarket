#ifndef LIBCRYPTOMARKET_H
#define LIBCRYPTOMARKET_H

#include <list>
#include <string>

using std::string;

namespace libcryptomarket {

typedef unsigned long long timestamp_t;

class BaseLogger
{
public:
    enum Level {
        Critical = -2,
        Warning = -1,
        Info = 0
    };

    BaseLogger() { }
    virtual void Log(Level lv, const string& message) = 0;
};

class ConsoleLogger : public BaseLogger
{
public:
    void Log(Level lv, const std::string &msg);
};

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

    void SetExchange(const string& name);
    void SetPriceStep(double step);
    void SetQtyStep(double step);

    int GetPricePrecision();
    int GetQtyPrecision();
    const string& GetExchange();

    int QuotePrecison;
    Asset Base;
    Asset Quote;

private:
    bool Valid_;
    double PriceStep_;
    double QtyStep_;
    string Exchange_;
};

struct Ticker24h {
    string Symbol;
    string Exchange;
    double Open;
    double High;
    double Low;
    double LastPrice;
    double Volume;
    double QuoteVolume;
};

struct Depth {
    enum DepthType {
        New = 0,
        Update = 1,
        Remove = 2
    };
    DepthType Type;
    double Price;
    double Qty;
};

enum TimeFrame {
    TimeFrame_1m = 0,
    TimeFrame_5m = 1,
    TimeFrame_15m = 2,
    TimeFrame_1h = 3,
    TimeFrame_4h = 4,
    TimeFrame_1d = 5,
};

enum Direct {
    Buy = 0,
    Sell = 1,
    Both = 2
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

enum OrderType {
    Limit,
    Market,
    StopLoss,
    TakeProfit,
    StopLossMarket,
    TakeProfitMarket
};

enum OrderStatus {
    New,
    Filled,
    Canceled
};

struct Order {
    Direct Side;
    std::string Symbol;
    double Qty;
    double Price;
    double StopPrice;
    timestamp_t Time;
    timestamp_t UpdateTime;
    unsigned long long Id;
    OrderType Type;
    OrderStatus Status;
};

enum MarginType {
    Isolated,
    Crossed
};

struct MarkPrice {
    double MarkPrice;
    double IndexPrice;
    std::string Symbol;
    timestamp_t Time;
};

struct FuturesMarginOption {
    MarginType Type;
    double Leverage;
};

struct Position {
    Position() {
        Qty = 0.0;
        MarkPrice = 0.0;
        EntryPrice = 0.0;
        LiquidationPrice = 0.0;
        UnrealizedProfit = 0.0;
    }
    MarginType Type;
    double Leverage;
    std::string Symbol;
    Direct Side;
    double Qty;
    double MarkPrice;
    double EntryPrice;
    double LiquidationPrice;
    double UnrealizedProfit;
    timestamp_t UpdateTime;
};

typedef std::list<Depth> MarketDepth;
typedef std::list<Trade> TradesList;
typedef std::list<Candle> CandlesList;
typedef std::list<Order> OrderList;

struct CryptoMarketHandle {
    void* ExchangeObj;
    string ExchangeName;
};

CryptoMarketHandle NewExchangeObj(const string& name, const std::string &api = "", const std::string &secret = "");
timestamp_t GetServerTime(CryptoMarketHandle& h);

bool SetExchangeObjLogger(CryptoMarketHandle handle, BaseLogger* logger);
bool GetSymbols(CryptoMarketHandle &h, std::list<Symbol> &symbols);
bool GetTicker24(CryptoMarketHandle &h, std::list<Ticker24h>& tickers);
bool GetMarketDepth(CryptoMarketHandle &h, const string &symbol, int limit, MarketDepth& Asks, MarketDepth& Bids);
bool GetTrades(CryptoMarketHandle &h, const string& symbol, timestamp_t start_time, timestamp_t end_time, int limit, TradesList& trades);
bool GetCandles(CryptoMarketHandle &h, const string& symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit, CandlesList& candles);
bool GetAccount(CryptoMarketHandle &h, AccountInfo& info);
bool GetOpenOrders(CryptoMarketHandle &h, OrderList& orders);
bool NewOrder(CryptoMarketHandle &h, std::string &symbol, OrderType type, Direct direct, double qty, double price, double stopPrice, Order& newOrder);
bool CancelOrder(CryptoMarketHandle &h, Order& order);
bool GetCurrentPosition(CryptoMarketHandle &h, const std::string &symbol, std::list<Position>& pos);
int GetErrorCode(CryptoMarketHandle &h);
std::string GetErrorMessage(CryptoMarketHandle &h);

// Futures only
bool GetFuturesMarginOption(CryptoMarketHandle &h, std::string& symbol, FuturesMarginOption& options);
bool SetFuturesMarginOption(CryptoMarketHandle &h, std::string& symbol, FuturesMarginOption& options);

bool GetListenKey(CryptoMarketHandle &h, std::string& key);
bool PutListenKey(CryptoMarketHandle &h, const std::string& key);
bool CloseListenKey(CryptoMarketHandle &h, const std::string& key);

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
static const int MARK_PRICE_SUBSCRIBE = 0x100;
static const int ALL_SUBSCRIBE = MARK_PRICE_SUBSCRIBE|MARKET_DEPTH_SUBSCRIBE|TRADES_SUBSCRIBE|
        CANDLES_SUBSCRIBE_1m|CANDLES_SUBSCRIBE_5m|CANDLES_SUBSCRIBE_15m|
        CANDLES_SUBSCRIBE_1h|CANDLES_SUBSCRIBE_4h|CANDLES_SUBSCRIBE_1d;

typedef void* WebSocketObj;
typedef void (*UpdateMarketDepthEvent)(void*, const std::string&, const std::string&, MarketDepth& asks, MarketDepth& bids);
typedef void (*AddTradeEvent)(void*, const std::string&, const std::string&, Trade& trade);
typedef void (*UpdateCandleEvent)(void*, const std::string&, const std::string&, TimeFrame tf, Candle& candle);
typedef void (*UpdateBalanceEvent)(void*, const std::string&, const std::string&, Balances& balances);
typedef void (*UpdatePositionEvent)(void*, const std::string&, const std::string&, std::list<Position>& position);
typedef void (*UpdateOrderEvent)(void*, const std::string&, const std::string&, Order& order);
typedef void (*UpdateMarkPriceEvent)(void*, const std::string&, const std::string&, MarkPrice& price);

WebSocketObj CreateWebSocketObj(const std::string& exchange, const std::string& symbol, int subscribe_flags = ALL_SUBSCRIBE, const std::string& listen_key = "");
bool SetWebSocketLogger(WebSocketObj ws, BaseLogger* logger);
bool SetWebSocketContext(WebSocketObj ws, void *context);
void StartWebSocket(WebSocketObj ws);
void StopWebSocket(WebSocketObj ws);
bool SetWebSocketUpdateMarketDepthCallback(WebSocketObj ws, UpdateMarketDepthEvent event);
bool SetWebSocketAddTradeCallback(WebSocketObj ws, AddTradeEvent event);
bool SetWebSocketUpdateCandleCallback(WebSocketObj ws, UpdateCandleEvent);
bool SetWebSocketUpdateBalanceCallback(WebSocketObj ws, UpdateBalanceEvent event);
bool SetWebSocketUpdatePositionCallback(WebSocketObj ws, UpdatePositionEvent event);
bool SetWebSocketUpdateOrderCallback(WebSocketObj ws, UpdateOrderEvent event);
bool SetWebSocketUpdateMarkPriceCallback(WebSocketObj ws, UpdateMarkPriceEvent event);
bool DeleteWebSocket(WebSocketObj ws);

};
#endif // LIBCRYPTOMARKET_H
