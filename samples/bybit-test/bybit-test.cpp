#include <libcryptomarket.h>
#include <iostream>
#include <thread>

using namespace libcryptomarket;
using namespace std;

static void AddTrade(void*, const std::string&, const std::string& symbol, Trade& trade)
{
    //std::cout << "Trade " << "Symbol: " << symbol << " Price: " << trade.Price << " Qty: " << trade.Qty << "\n";
}

static void UpdateCandle(void*, const std::string&, const std::string& symbol, TimeFrame tf, Candle& candle)
{
    //std::cout << "Candle " << "Symbol: " << symbol << " C: " << candle.Close << " O: " << candle.Open << "\n";
}

static void UpdateMarketDepth(void*, const std::string&, const std::string& symbol, MarketDepthSeries& series)
{
    //std::cout << "Market Depth" << " Symbol: " << symbol << " Series count: " << series.Items.size() << "\n";
}

int main()
{
    ConsoleLogger Logger;
    CryptoMarketHandle Exchange = NewExchangeObj("bybit");
    std::list<Symbol> Symbols;
    //SetExchangeObjLogger(Exchange, &Logger);
    GetSymbols(Exchange, Symbols);
    Prices prices;
    GetAllPrices(Exchange, prices);
    std::cout << "Prices count: " << prices.size() << "\n";
    std::cout << "ServerTime: " << GetServerTime(Exchange) << "\n";
    std::cout << "Symbols count: " << Symbols.size() << "\n";

    MarketDepth asks;
    MarketDepth bids;
    uint64_t id;
    GetMarketDepth(Exchange, "BTCUSDT", 100, asks, bids, id);
    std::cout << "Asks count " << asks.size() << " Bids count " << bids.size() << "\n";

    TradesList trades;
    GetTrades(Exchange, "BTCUSDT", -1, -1, 100, trades);
    std::cout << "Trades count " << trades.size() << "\n";

    CandlesList candles;
    GetCandles(Exchange, "BTCUSDT", TimeFrame_5m, 0, 0, 0, candles);
    std::cout << "Candles count " << candles.size() << "\n";
    /*WebSocketObj Handle;
    Handle = CreateWebSocketObj("bybit", "BTCUSDT", ALL_SUBSCRIBE);
    SetWebSocketLogger(Handle, &Logger);
    SetWebSocketAddTradeCallback(Handle, AddTrade);
    SetWebSocketUpdateCandleCallback(Handle, UpdateCandle);
    SetWebSocketUpdateMarketDepthCallback(Handle, UpdateMarketDepth);
    StartWebSocket(Handle);
    std::this_thread::sleep_for(std::chrono::seconds(10));

    StopWebSocket(Handle);
    DeleteWebSocket(Handle);*/
    return 0;
}
