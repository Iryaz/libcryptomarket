#include <boost/format.hpp>
#include "binanceexchange.h"
#include "binancefuturesexchange.h"
#include <iostream>
#include "websocket/binancewebsocket.h"

#define F(S) boost::format(S)

BinanceWebSocket::BinanceWebSocket(Type type, const std::string& symbol, int subscribe_flags) :
    BaseWebSocket(type, symbol, subscribe_flags)
{
    LastUpdateId_ = -1;
    SetSymbol(symbol);
    switch (Type_) {
    case Spot: {
        BaseWebSocket::SetWebSocketPort(9443);
        SetHost("stream.binance.com");
        Exchange_ = "binance";
        SetPath("/ws/");
        BinanceObj_ = new BinanceExchange();
    } break;
    case Futures: {
        BaseWebSocket::SetWebSocketPort(443);
        SetHost("fstream.binance.com");
        Exchange_ = "binance-futures";
        SetPath("/stream?streams=");
        BinanceObj_ = new BinanceFuturesExchange();
    } break;
    default:
        break;
    }

    Init(GetSubscribeFlags());
    BinanceObj_->Init("", "");
}

BinanceWebSocket::~BinanceWebSocket()
{
    if (BinanceObj_ != nullptr)
        delete BinanceObj_;
}

bool BinanceWebSocket::StartLoop()
{
    if (GetSubcribeFlags() & MARKET_DEPTH_SUBSCRIBE) {
            MarketDepthSeries asks;
            MarketDepthSeries bids;
            BinanceObj_->GetMarketDepth(GetSymbol(), 100, asks.Items, bids.Items, LastUpdateId_);
            if (UpdateMarketDepthCallback_ != nullptr) {
                UpdateMarketDepthCallback_(Context_, Exchange_, GetSymbol(), bids);
                UpdateMarketDepthCallback_(Context_, Exchange_, GetSymbol(), asks);
            }
    }
    return BaseWebSocket::StartLoop();
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
        timestamp_t eventTime  = json.at("E").to_number<timestamp_t>();
        std::string symbol = json.at("s").as_string().c_str();
        MarketDepthSeries BidsDepth;
        MarketDepthSeries AsksDepth;
        uint64_t FirstUpdateId = json.at("U").to_number<uint64_t>();
        uint64_t FinalUpdateId = json.at("u").to_number<uint64_t>();

        if (FirstUpdateId <= LastUpdateId_+1 && FinalUpdateId >= LastUpdateId_+1)
            LastUpdateId_ = FinalUpdateId;

        BidsDepth.UpdateTime = eventTime;
        AsksDepth.UpdateTime = eventTime;

        BidsDepth.IsBids = true;
        AsksDepth.IsBids = false;

        for (auto b : json.at("b").as_array()) {
            double price = atof(b.at(0).as_string().c_str());
            double qty 	 = atof(b.at(1).as_string().c_str());
            Depth item;
            item.Type = Depth::Update;
            item.Price = price;
            item.Qty = qty;
            if (item.Qty <= 0)
                item.Type = Depth::Remove;
            if (FinalUpdateId < LastUpdateId_)
                item.Type = Depth::Remove;
            BidsDepth.Items.push_back(item);
            if (UpdateMarketDepthCallback_ != nullptr)
                UpdateMarketDepthCallback_(Context_, Exchange_, symbol, BidsDepth);
        }

        for (auto a : json.at("a").as_array()) {
            double price = atof(a.at(0).as_string().c_str());
            double qty 	 = atof(a.at(1).as_string().c_str());
            Depth item;
            item.Type = Depth::Update;
            item.Price = price;
            item.Qty = qty;
            if (item.Qty <= 0)
                item.Type = Depth::Remove;
            if (FinalUpdateId < LastUpdateId_)
                item.Type = Depth::Remove;
            AsksDepth.Items.push_back(item);
            if (UpdateMarketDepthCallback_ != nullptr)
                UpdateMarketDepthCallback_(Context_, Exchange_, symbol, AsksDepth);
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
