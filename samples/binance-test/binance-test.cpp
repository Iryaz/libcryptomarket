#include <libcryptomarket.h>
#include <iostream>
#include <thread>

using namespace libcryptomarket;
using namespace std;

static void AddTrade(void*, const std::string&, const std::string& symbol, Trade& trade)
{
    std::cout << "Trade " << "Symbol: " << symbol << " Price: " << trade.Price << " Qty: " << trade.Qty << "\n";
}

static void UpdateCandle(void*, const std::string&, const std::string& symbol, TimeFrame tf, Candle& candle)
{
    std::cout << "Candle " << "Symbol: " << symbol << " C: " << candle.Close << " O: " << candle.Open << "\n";
}

static void UpdateMarketDepth(void*, const std::string&, const std::string& symbol, MarketDepth& asks, MarketDepth& bids)
{
    std::cout << "Market depth " << "Symbol: " << symbol << "\n";
}

int main()
{
    ConsoleLogger Logger;
    std::list<Symbol> info;
    CryptoMarketHandle Exchange = NewExchangeObj("binance-futures");
    SetExchangeObjLogger(Exchange, &Logger);
    std::cout << "Server time: " << GetServerTime(Exchange) << "\n";
    GetSymbols(Exchange, info);
    std::cout << "Exchange symbol count: " << info.size() << "\n";

    WebSocketObj Handle;
    Handle = CreateWebSocketObj("binance-futures", "BTCUSDT", MARKET_DEPTH_SUBSCRIBE);
    SetWebSocketLogger(Handle, &Logger);
    SetWebSocketAddTradeCallback(Handle, AddTrade);
    SetWebSocketUpdateCandleCallback(Handle, UpdateCandle);
    SetWebSocketUpdateMarketDepthCallback(Handle, UpdateMarketDepth);
    StartWebSocket(Handle);
    std::this_thread::sleep_for(std::chrono::seconds(50));

    StopWebSocket(Handle);
    DeleteWebSocket(Handle);
    return 0;
}
