#include "binancefuturesexchange.h"
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

    newHandle.ExchangeObj = nullptr;
    return newHandle;
}

timestamp_t GetServerTime(CryptoMarketHandle& h)
{
    return static_cast<ExchangeObj*>(h.ExchangeObj)->GetServerTime();
}

bool GetExchangeInfo(CryptoMarketHandle& h, ExchangeInfo& info)
{
    return static_cast<ExchangeObj*>(h.ExchangeObj)->GetExchangeInfo(info);
}

bool GetAllPrices(CryptoMarketHandle &h, Prices& prices)
{
    return static_cast<ExchangeObj*>(h.ExchangeObj)->GetAllPrices(prices);
}

bool GetMarketDepth(CryptoMarketHandle &h, const string &symbol, int limit, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId)
{
    return static_cast<ExchangeObj*>(h.ExchangeObj)->GetMarketDepth(symbol, limit, Asks, Bids, lastUpdateId);
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
        return new BinanceWebSocket(symbol, subscribe_flags);

    if (exchange == "bybit")
        return new ByBitWebsocket(symbol, subscribe_flags);

    return nullptr;
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

    if (static_cast<BaseWebSocket *>(ws)->GetExchange() == "binance") {
        static_cast<BinanceWebSocket *>(ws)->Stop();
        delete static_cast<BinanceWebSocket *>(ws);
    }

    if (static_cast<BaseWebSocket *>(ws)->GetExchange() == "bybit") {
        static_cast<ByBitWebsocket *>(ws)->Stop();
        delete static_cast<ByBitWebsocket *>(ws);
    }

    return true;
}

}
