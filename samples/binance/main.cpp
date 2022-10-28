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

int main()
{
    ConsoleLogger Logger;
    ExchangeInfo info;
    CryptoMarketHandle Exchange = NewExchangeObj("binance");
    SetExchangeObjLogger(Exchange, &Logger);
    std::cout << "Server time: " << GetServerTime(Exchange) << "\n";
    GetExchangeInfo(Exchange, info);
    std::cout << "Exchange symbol count: " << info.Symbols.size() << "\n";

    WebSocketObj Handle;
    Handle = CreateWebSocketObj("binance", "BTCUSDT", TRADES_SUBSCRIBE|CANDLES_SUBSCRIBE_1m|CANDLES_SUBSCRIBE_5m);
    SetWebSocketLogger(Handle, &Logger);
    SetWebSocketAddTradeCallback(Handle, AddTrade);
    SetWebSocketUpdateCandleCallback(Handle, UpdateCandle);
    StartWebSocket(Handle);
    std::this_thread::sleep_for(std::chrono::seconds(10));

    StopWebSocket(Handle);
    DeleteWebSocket(Handle);
    return 0;
}
