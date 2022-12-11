#include <boost/format.hpp>
#include "binanceexchange.h"
#include "binancefuturesexchange.h"
#include <iostream>
#include "websocket/binancewebsocket.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define F(S) boost::format(S)

BinanceWebSocket::BinanceWebSocket(Type type, const std::string& symbol, int subscribe_flags, const std::string& listen_key) :
    BaseWebSocket(type, symbol, subscribe_flags)
{
    LastUpdateId_ = -1;
    SetSymbol(symbol);
    switch (Type_) {
    case Spot: {
        BaseWebSocket::SetWebSocketPort(9443);
        if (listen_key.empty())
            SetHost("stream.binance.com");
        else
            SetHost("stream-auth.binance.com");
        Exchange_ = "binance";
        SetPath("/stream?streams=");
        BinanceObj_ = new BinanceExchange();
    } break;
    case Futures: {
        BaseWebSocket::SetWebSocketPort(443);
        if (listen_key.empty())
            SetHost("fstream.binance.com");
        else
            SetHost("fstream.binance.com");
        Exchange_ = "binance-futures";
        SetPath("/stream");
        BinanceObj_ = new BinanceFuturesExchange();
    } break;
    default:
        break;
    }

    Init(GetSubscribeFlags(), listen_key);
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
            MarketDepth asks;
            MarketDepth bids;
            BinanceObj_->GetMarketDepth(GetSymbol(), 100, asks, bids, LastUpdateId_);
            if (UpdateMarketDepthCallback_ != nullptr)
                UpdateMarketDepthCallback_(Context_, Exchange_, GetSymbol(), asks, bids);
    }

    return BaseWebSocket::StartLoop();
}

void BinanceWebSocket::Init(int flag, const std::string& listen_key)
{
    boost::property_tree::ptree pt;
    boost::property_tree::ptree args;
    boost::property_tree::ptree params;

    if (flag & MARKET_DEPTH_SUBSCRIBE) {
        boost::property_tree::ptree arg;
        arg.put("", GetSymbol() + "@depth@100ms");
        params.push_back(std::make_pair("", arg));
    }
    if (flag & TRADES_SUBSCRIBE) {
        boost::property_tree::ptree arg;
        arg.put("", GetSymbol() + "@aggTrade");
        params.push_back(std::make_pair("", arg));
    }
    if (flag & CANDLES_SUBSCRIBE_1m) {
        boost::property_tree::ptree arg;
        arg.put("", GetSymbol() + "@kline_1m");
        params.push_back(std::make_pair("", arg));
    }
    if (flag & CANDLES_SUBSCRIBE_5m) {
        boost::property_tree::ptree arg;
        arg.put("", GetSymbol() + "@kline_5m");
        params.push_back(std::make_pair("", arg));
    }
    if (flag & CANDLES_SUBSCRIBE_15m) {
        boost::property_tree::ptree arg;
        arg.put("", GetSymbol() + "@kline_15m");
        params.push_back(std::make_pair("", arg));
    }
    if (flag & CANDLES_SUBSCRIBE_1h) {
        boost::property_tree::ptree arg;
        arg.put("", GetSymbol() + "@kline_1h");
        params.push_back(std::make_pair("", arg));
    }
    if (flag & CANDLES_SUBSCRIBE_4h) {
        boost::property_tree::ptree arg;
        arg.put("", GetSymbol() + "@kline_4h");
        params.push_back(std::make_pair("", arg));
    }
    if (flag & CANDLES_SUBSCRIBE_1d) {
        boost::property_tree::ptree arg;
        arg.put("", GetSymbol() + "@kline_1d");
        params.push_back(std::make_pair("", arg));
    }
    if (!listen_key.empty()) {
        boost::property_tree::ptree arg;
        arg.put("", listen_key);
        params.push_back(std::make_pair("", arg));
    }

    pt.put("id", 1);
    pt.put("method", "SUBSCRIBE");
    pt.add_child("params", params);

    std::stringstream ss;
    boost::property_tree::json_parser::write_json(ss, pt);
    Message_ = ss.str();
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
        case MARGIN_CALL:
            ParseMarginCall(result);
            break;
        case ACCOUNT_UPDATE:
            ParseAccountUpdate(result);
            break;
        case ORDER_TRADE_UPDATE:
            ParseOrderTrade(result);
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
        case MARGIN_CALL:
            ParseMarginCall(obj);
            break;
        case ACCOUNT_UPDATE:
            ParseAccountUpdate(obj);
            break;
        case ORDER_TRADE_UPDATE:
            ParseOrderTrade(obj);
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
    if (s == "MARGIN_CALL")
        return MARGIN_CALL;
    if (s == "ACCOUNT_UPDATE")
        return ACCOUNT_UPDATE;
    if (s == "ORDER_TRADE_UPDATE")
        return ORDER_TRADE_UPDATE;

    return UNKNOWN;
}

void BinanceWebSocket::ParseMarketDepth(const json::value& json)
{
    try {
        timestamp_t eventTime  = json.at("E").to_number<timestamp_t>();
        std::string symbol = json.at("s").as_string().c_str();
        MarketDepth Bids;
        MarketDepth Asks;
        uint64_t FirstUpdateId = json.at("U").to_number<uint64_t>();
        uint64_t FinalUpdateId = json.at("u").to_number<uint64_t>();

        //if (FirstUpdateId <= LastUpdateId_+1 && FinalUpdateId >= LastUpdateId_+1)
        //    LastUpdateId_ = FinalUpdateId;

        for (auto& b : json.at("b").as_array()) {
            double price = atof(b.at(0).as_string().c_str());
            double qty 	 = atof(b.at(1).as_string().c_str());
            Depth item;
            item.Type = Depth::Update;
            item.Price = price;
            item.Qty = qty;
            if (item.Qty <= 0)
                item.Type = Depth::Remove;
            if (FinalUpdateId <= LastUpdateId_)
                item.Type = Depth::Remove;
            Bids.push_back(item);
        }

        for (auto& a : json.at("a").as_array()) {
            double price = atof(a.at(0).as_string().c_str());
            double qty 	 = atof(a.at(1).as_string().c_str());
            Depth item;
            item.Type = Depth::Update;
            item.Price = price;
            item.Qty = qty;
            if (item.Qty <= 0)
                item.Type = Depth::Remove;
            if (FinalUpdateId <= LastUpdateId_)
                item.Type = Depth::Remove;
            Asks.push_back(item);
        }

        if (UpdateMarketDepthCallback_ != nullptr)
            UpdateMarketDepthCallback_(Context_, Exchange_, symbol, Asks, Bids);

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

void BinanceWebSocket::ParseMarginCall(const json::value& json)
{

}

void BinanceWebSocket::ParseAccountUpdate(const json::value& json)
{
    try {
        Balances balances;
        auto& obj = json.at("a").as_object();
        auto& B = obj.at("B").as_array();
        for (auto& b : B) {
            Balance balance;
            balance.Asset = b.at("a").as_string().c_str();
            double balance_val = atof(b.at("wb").as_string().c_str());
            balance.Locked = balance_val - atof(b.at("cw").as_string().c_str());
            balance.Free = atof(b.at("wb").as_string().c_str()) - balance.Locked;
            balances.push_back(balance);
        }

        if (UpdateBalanceCallback_ != nullptr)
            UpdateBalanceCallback_(Context_, Exchange_, Symbol_, balances);

    } catch (std::exception& e) {
        ErrorMessage((F("<BinanceWebSocket::ParseAccountUpdate> Error parse Account: %s") % e.what()).str());
    }
}

void BinanceWebSocket::ParseOrderTrade(const json::value& json)
{

}
