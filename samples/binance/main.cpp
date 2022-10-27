#include <./libcryptomarket.h>
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

int main()
{
    WebSocketObj Handle;
    Handle = CreateWebSocketObj("binance", "BTCUSDT", TRADES_SUBSCRIBE|CANDLES_SUBSCRIBE_1m|CANDLES_SUBSCRIBE_5m);
    SetWebSocketAddTradeCallback(Handle, AddTrade);
    SetWebSocketUpdateCandleCallback(Handle, UpdateCandle);
    StartWebSocket(Handle);
    std::this_thread::sleep_for(std::chrono::seconds(60));

    StopWebSocket(Handle);
    DeleteWebSocket(Handle);
    return 0;
}
