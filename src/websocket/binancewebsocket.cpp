#include <iostream>
#include "websocket/binancewebsocket.h"
#include <boost/format.hpp>

#define F(S) boost::format(S)

BinanceWebSocket::BinanceWebSocket(Type type, const std::string& symbol, int subscribe_flags) :
    BaseWebSocket(type, symbol, subscribe_flags)
{
    SetSymbol(symbol);
    switch (Type_) {
    case Spot: {
        SetPort(9443);
        SetHost("stream.binance.com");
        Exchange_ = "binance";
        SetPath("/ws/");
    } break;
    case Futures: {
        SetPort(443);
        SetHost("fstream.binance.com");
        Exchange_ = "binance-futures";
        SetPath("/stream?streams=");
    } break;
    default:
        break;
    }

    Init(GetSubscribeFlags());
}

void BinanceWebSocket::Init(int flag)
{
    if (flag & MARKET_DEPTH_SUBSCRIBE)
        PathParams_ += GetSymbol() + "@depth@100ms" + "/";
    if (flag & TRADES_SUBSCRIBE)
        PathParams_ += GetSymbol() + "@aggTrade" + "/";
    if (flag & CANDLES_SUBSCRIBE_1m)
        PathParams_ += GetSymbol() + "@kline_1m" + "/";
    if (flag & CANDLES_SUBSCRIBE_5m)
        PathParams_ += GetSymbol() + "@kline_5m" + "/";
    if (flag & CANDLES_SUBSCRIBE_15m)
        PathParams_ += GetSymbol() + "@kline_15m" + "/";
    if (flag & CANDLES_SUBSCRIBE_1h)
        PathParams_ += GetSymbol() + "@kline_1h" + "/";
    if (flag & CANDLES_SUBSCRIBE_4h)
        PathParams_ += GetSymbol() + "@kline_4h" + "/";
    if (flag & CANDLES_SUBSCRIBE_1d)
        PathParams_ += GetSymbol() + "@kline_1d" + "/";

    PathParams_.pop_back();
    SetPath(Path_ + PathParams_);
}

void BinanceWebSocket::SetSymbol(const std::string& symbol)
{
    Symbol_ = symbol;
    std::transform(Symbol_.begin(), Symbol_.end(), Symbol_.begin(), [](unsigned char c) {return std::tolower(c);});
}

void BinanceWebSocket::ParseJSon(boost::json::value& result)
{
    switch (Type_) {
    case Spot:
        ParseDataSpot(result);
        break;
    case Futures:
        ParseDataFutures(result);
        break;
    }
}

void BinanceWebSocket::ParseDataSpot(boost::json::value& result)
{
    try {
        DataEventType event = String2EventType(result.at("e").as_string().c_str());
        switch (event) {
        case UNKNOWN:
            break;
        case DEPTH_UPDATE:
            ParseMarketDepth(result);
            break;
        case AGG_TRADE:
            ParseTrades(result);
            break;
        case KLINE:
            ParseKLines(result);
            break;
        }
    } catch (std::exception& e) {
        ErrorMessage((F("<BinanceWebSocket::ParseDataSpot> Error parsing json: %s") % e.what()).str());
    }
}

void BinanceWebSocket::ParseDataFutures(boost::json::value& result)
{
    try {
        auto obj = result.at("data").as_object();
        DataEventType event = String2EventType(obj.at("e").as_string().c_str());
        switch (event) {
        case UNKNOWN:
            break;
        case DEPTH_UPDATE:
            ParseMarketDepth(obj);
            break;
        case AGG_TRADE:
            ParseTrades(obj);
            break;
        case KLINE:
            ParseKLines(obj);
            break;
        }
    } catch (std::exception& e) {
        ErrorMessage((F("<BinanceWebSocket::ParseDataFutures> Error parsing json: %s") % e.what()).str());
    }
}

BaseWebSocket::DataEventType BinanceWebSocket::String2EventType(const std::string& s)
{
    if (s == "depthUpdate")
        return DEPTH_UPDATE;
    if (s == "aggTrade")
        return AGG_TRADE;
    if (s == "kline")
        return KLINE;

    return UNKNOWN;
}

void BinanceWebSocket::ParseMarketDepth(const json::value& json)
{
    try {
        timestamp_t new_updateId  = json.at("u").as_int64();
        std::string symbol = json.at("s").as_string().c_str();
        MarketDepthSeries BidsDepth;
        MarketDepthSeries AsksDepth;

        BidsDepth.UpdateTime = new_updateId;
        AsksDepth.UpdateTime = new_updateId;

        BidsDepth.IsBids = true;
        AsksDepth.IsBids = false;

        for (auto b : json.at("b").as_array()) {
            double price = atof(b.at(0).as_string().c_str());
            double qty 	 = atof(b.at(1).as_string().c_str());
            Depth item;
            item.Price = price;
            item.Qty = qty;
            BidsDepth.Items.push_back(item);
            if (UpdateMarketDepthCallback_ != nullptr)
                UpdateMarketDepthCallback_(Context_, Exchange_, symbol, BidsDepth);
        }

        for (auto a : json.at("a").as_array()) {
            double price = atof(a.at(0).as_string().c_str());
            double qty 	 = atof(a.at(1).as_string().c_str());
            Depth item;
            item.Price = price;
            item.Qty = qty;
            AsksDepth.Items.push_back(item);
            if (UpdateMarketDepthCallback_ != nullptr)
                UpdateMarketDepthCallback_(Context_, Exchange_, symbol, BidsDepth);
        }
    } catch (std::exception& e) {
        ErrorMessage((F("<BinanceWebSocket::ParseMarketDepth> Error parsing json: %s") % e.what()).str());
    }
}

void BinanceWebSocket::ParseTrades(const json::value& json)
{
    try {
        long long aggTradeId = json.at("a").to_number<uint64_t>();
        std::string symbol = json.at("s").as_string().c_str();
        Trade trade;
        trade.Id = aggTradeId;
        trade.Price = atof(json.at("p").as_string().c_str());
        trade.Qty   = atof(json.at("q").as_string().c_str());
        trade.Time  = json.at("T").to_number<uint64_t>();
        trade.IsBuy = json.at("m").as_bool();

        if (AddTradeCallback_ != nullptr)
            AddTradeCallback_(Context_, Exchange_, symbol, trade);
    } catch (std::exception& e) {
        ErrorMessage((F("<BinanceWebSocket::ParseTrades> Error parse trades: %s") % e.what()).str());
    }
}

void BinanceWebSocket::ParseKLines(const json::value &json)
{
    try {
        auto& k = json.at("k");
        std::string symbol = json.at("s").as_string().c_str();
        unsigned long long start_of_candle = k.at("t").to_number<uint64_t>();
        std::string tfs = k.at("i").as_string().c_str();
        TimeFrame tf = GetTimeFrame(tfs);
        Candle c;
        c.OpenTime = start_of_candle;
        c.CloseTime = k.at("T").to_number<uint64_t>();
        c.Open = atof(k.at("o").as_string().c_str());
        c.High = atof(k.at("h").as_string().c_str());
        c.Low = atof(k.at("l").as_string().c_str());
        c.Close = atof(k.at("c").as_string().c_str());
        c.Qty = atof(k.at("v").as_string().c_str());

        if (UpdateCandleCallback_ != nullptr)
            UpdateCandleCallback_(Context_, Exchange_, symbol, tf, c);
    } catch (std::exception& e) {
        ErrorMessage((F("<BinanceWebSocket::ParseKLines> Error parse candles: %s") % e.what()).str());
    }
}
