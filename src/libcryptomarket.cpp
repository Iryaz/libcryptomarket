#include "binancefuturesexchange.h"
#include "bybitexchange.h"
#include "bybitfuturesexchange.h"
#include "exchangeobj.h"
#include "websocket/binancewebsocket.h"
#include "websocket/bybitwebsocket.h"

#include <boost/json/src.hpp>
#include "libcryptomarket.h"

namespace libcryptomarket {

static std::list<ExchangeObj*> Objects;

CryptoMarketHandle NewExchangeObj(const string& name, const string& api, const string& secret)
{
    CryptoMarketHandle newHandle;
    if (name == "binance") {
        BinanceExchange* ex = new BinanceExchange();
        newHandle.ExchangeName = "binance";
        newHandle.ExchangeObj = ex;
        ex->Init(api, secret);
        Objects.push_back(ex);
        return newHandle;
    }

    if (name == "binance-futures") {
        BinanceFuturesExchange* ex = new BinanceFuturesExchange();
        newHandle.ExchangeName = "binance-futures";
        newHandle.ExchangeObj = ex;
        ex->Init(api, secret);
        Objects.push_back(ex);
        return newHandle;
    }

    if (name == "bybit") {
        BybitExchange* ex = new BybitExchange();
        newHandle.ExchangeName = "bybit";
        newHandle.ExchangeObj = ex;
        ex->Init(api, secret);
        Objects.push_back(ex);
        return newHandle;
    }

    if (name == "bybit-futures") {
        BybitFuturesExchange* ex = new BybitFuturesExchange();
        newHandle.ExchangeName = "bybit-futures";
        newHandle.ExchangeObj = ex;
        ex->Init(api, secret);
        Objects.push_back(ex);
        return newHandle;
    }

    newHandle.ExchangeObj = nullptr;
    return newHandle;
}

timestamp_t GetServerTime(CryptoMarketHandle& h)
{
    return static_cast<ExchangeObj*>(h.ExchangeObj)->GetServerTime();
}

bool SetExchangeObjLogger(CryptoMarketHandle handle, BaseLogger* logger)
{
    if (handle.ExchangeName == "binance") {
        static_cast<BinanceExchange*>(handle.ExchangeObj)->SetLogger(logger);
        return true;
    }

    if (handle.ExchangeName == "binance-futures") {
        static_cast<BinanceFuturesExchange*>(handle.ExchangeObj)->SetLogger(logger);
        return true;
    }

    if (handle.ExchangeName == "bybit") {
        static_cast<BybitExchange*>(handle.ExchangeObj)->SetLogger(logger);
        return true;
    }

    if (handle.ExchangeName == "bybit-futures") {
        static_cast<BybitFuturesExchange*>(handle.ExchangeObj)->SetLogger(logger);
        return true;
    }

    return false;
}

bool GetSymbols(CryptoMarketHandle& h, std::list<Symbol> &symbols)
{
    return static_cast<ExchangeObj*>(h.ExchangeObj)->GetSymbols(symbols);
}

bool GetTicker24(CryptoMarketHandle &h, std::list<Ticker24h>& tickers)
{
    return static_cast<ExchangeObj*>(h.ExchangeObj)->GetTicker24(tickers);
}

bool GetMarketDepth(CryptoMarketHandle &h, const string &symbol, int limit, MarketDepth& Asks, MarketDepth& Bids)
{
    uint64_t lasUpdateId = 0;
    return static_cast<ExchangeObj*>(h.ExchangeObj)->GetMarketDepth(symbol, limit, Asks, Bids, lasUpdateId);
}

bool GetTrades(CryptoMarketHandle &h, const string& symbol, timestamp_t start_time, timestamp_t end_time, int limit, TradesList& trades)
{
    return static_cast<ExchangeObj*>(h.ExchangeObj)->GetTrades(symbol, start_time, end_time, limit, trades);
}

bool GetCandles(CryptoMarketHandle &h, const string& symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit, CandlesList& candles)
{
    return static_cast<ExchangeObj*>(h.ExchangeObj)->GetCandles(symbol, tf, start_time, end_time, limit, candles);
}

bool GetAccount(CryptoMarketHandle &h, AccountInfo& info)
{
    return static_cast<ExchangeObj*>(h.ExchangeObj)->GetAccount(info);
}

bool GetOpenOrders(CryptoMarketHandle &h, OrderList& orders)
{
    return static_cast<ExchangeObj*>(h.ExchangeObj)->GetOpenOrders(orders);
}

bool Free(CryptoMarketHandle handle)
{
    ExchangeObj* Ptr = static_cast<ExchangeObj*>(handle.ExchangeObj);
    Objects.remove(Ptr);
    if (Ptr == nullptr)
        return false;
    delete Ptr;
    return true;
}

void Cleanup()
{
    for (auto o : Objects)
        delete o;

    Objects.clear();
}

WebSocketObj CreateWebSocketObj(const std::string& exchange, const std::string& symbol, int subscribe_flags)
{
    if (exchange == "binance")
        return new BinanceWebSocket(BaseWebSocket::Spot, symbol, subscribe_flags);

    if (exchange == "binance-futures")
        return new BinanceWebSocket(BaseWebSocket::Futures, symbol, subscribe_flags);

    if (exchange == "bybit")
        return new ByBitWebsocket(BaseWebSocket::Spot, symbol, subscribe_flags);

    if (exchange == "bybit-futures")
        return new ByBitWebsocket(BaseWebSocket::Futures, symbol, subscribe_flags);

    return nullptr;
}

bool SetWebSocketLogger(WebSocketObj ws, BaseLogger* logger)
{
    if (ws == nullptr) return false;
    static_cast<BaseWebSocket *>(ws)->SetLogger(logger);
    return true;
}

bool SetWebSocketContext(WebSocketObj ws, void *context)
{
    if (ws == nullptr) return false;
    static_cast<BaseWebSocket *>(ws)->SetContext(context);
    return true;
}

void StartWebSocket(WebSocketObj ws)
{
    static_cast<BaseWebSocket *>(ws)->StartThread();
}

void StopWebSocket(WebSocketObj ws)
{
    static_cast<BaseWebSocket *>(ws)->Stop();
}

bool SetWebSocketUpdateMarketDepthCallback(WebSocketObj ws, UpdateMarketDepthEvent event)
{
    if (ws == nullptr)
        return false;
    static_cast<BaseWebSocket *>(ws)->SetMarketDepthEvent(event);
    return true;
}

bool SetWebSocketAddTradeCallback(WebSocketObj ws, AddTradeEvent event)
{
    if (ws == nullptr)
        return false;
    static_cast<BaseWebSocket *>(ws)->SetAddTradeEvent(event);
    return true;
}

bool SetWebSocketUpdateCandleCallback(WebSocketObj ws, UpdateCandleEvent event)
{
    if (ws == nullptr)
        return false;
    static_cast<BaseWebSocket *>(ws)->SetUpdateCandleEvent(event);
    return true;
}

bool DeleteWebSocket(WebSocketObj ws)
{
    if (ws == nullptr)
        return false;

    auto name =  static_cast<BaseWebSocket *>(ws)->GetExchange();
    if (name == "binance" || name == "binance-futures") {
        static_cast<BinanceWebSocket *>(ws)->Stop();
        delete static_cast<BinanceWebSocket *>(ws);
    }

    if (name == "bybit" || name == "bybit-futures") {
        static_cast<ByBitWebsocket *>(ws)->Stop();
        delete static_cast<ByBitWebsocket *>(ws);
    }

    return true;
}

}
