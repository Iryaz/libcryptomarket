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
        SetHost("stream.binance.com");
        Exchange_ = "binance";
        SetPath("/stream");
        BinanceObj_ = new BinanceExchange();
    } break;
    case Futures: {
        BaseWebSocket::SetWebSocketPort(443);
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
    /*boost::property_tree::ptree pt;
    boost::property_tree::ptree args;
    boost::property_tree::ptree params;

    if (flag & MARK_PRICE_SUBSCRIBE) {
        boost::property_tree::ptree arg;
        arg.put("", GetSymbol() + "@markPrice@1s");
        params.push_back(std::make_pair("", arg));
    }

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

    //pt.put<unsigned int>("id", 1);
    pt.put("method", "SUBSCRIBE");
    pt.add_child("params", params);
    pt.put("id", 1);
    std::stringstream ss;
    boost::property_tree::json_parser::write_json(ss, pt);
    Message_ = ss.str();*/

    Message_ += "{\n";
    Message_ += "\"id\": 1,\n";
    Message_ += "\"method\": \"SUBSCRIBE\",\n";
    Message_ += "\"params\": [\n";

    if (flag & MARK_PRICE_SUBSCRIBE) {
        Message_ += "\"" + GetSymbol() + "@markPrice@1s\",\n";
    }

    if (flag & MARKET_DEPTH_SUBSCRIBE) {
        Message_ += "\"" + GetSymbol() + "@depth@100ms\",\n";
    }

    if (flag & TRADES_SUBSCRIBE) {
        Message_ += "\"" + GetSymbol() + "@aggTrade\",\n";
    }
    if (flag & CANDLES_SUBSCRIBE_1m) {
        Message_ += "\"" + GetSymbol() + "@kline_1m\",\n";
    }

    if (flag & CANDLES_SUBSCRIBE_5m) {
        Message_ += "\"" + GetSymbol() + "@kline_5m\",\n";
    }
    if (flag & CANDLES_SUBSCRIBE_15m) {
        Message_ += "\"" + GetSymbol() + "@kline_15m\",\n";
    }
    if (flag & CANDLES_SUBSCRIBE_1h) {
        Message_ += "\"" + GetSymbol() + "@kline_1h\",\n";
    }
    if (flag & CANDLES_SUBSCRIBE_4h) {
        Message_ += "\"" + GetSymbol() + "@kline_4h\",\n";
    }
    if (flag & CANDLES_SUBSCRIBE_1d) {
        Message_ += "\"" + GetSymbol() + "@kline_1d\",\n";
    }
    if (!listen_key.empty()) {
        Message_ += "\"" + listen_key + "\",\n";
    }
    if (Message_.at(Message_.size()-2) == ',') {
        Message_.at(Message_.size()-2) = '\n';
        Message_.at(Message_.size()-1) = ' ';
    }
    Message_ += "]\n}";
}

void BinanceWebSocket::SetSymbol(const std::string& symbol)
{
    Symbol_ = symbol;
    std::transform(Symbol_.begin(), Symbol_.end(), Symbol_.begin(), [](unsigned char c) {return std::tolower(c);});
}

void BinanceWebSocket::ParseJSon(boost::json::value& result)
{
    if (IsNull(result))
        return;

    switch (Type_) {
    case Spot:
        ParseDataSpot(result);
        break;
    case Futures:
        ParseDataFutures(result);
        break;
    }
}

bool BinanceWebSocket::IsNull(boost::json::value& val)
{
    try {
        if (val.at("result").is_null())
            return true;
    } catch (std::exception& e) {
        return false;
    }

    return false;
}

void BinanceWebSocket::ParseDataSpot(boost::json::value& result)
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
        case MARK_PRICE:
            ParseMarkPrice(obj);
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
        case MARK_PRICE:
            ParseMarkPrice(obj);
            break;
        }
    } catch (std::exception& e) {
        ErrorMessage((F("<BinanceWebSocket::ParseDataFutures> Error parsing json: %s") % e.what()).str());
    }
}

BaseWebSocket::DataEventType BinanceWebSocket::String2EventType(const std::string& s)
{
    if (s == "markPriceUpdate")
        return MARK_PRICE;
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

Direct BinanceWebSocket::String2Direct(const std::string& s)
{
    if (s == "BOTH")
        return Direct::Both;
    if (s == "SELL")
        return Direct::Sell;

    return Direct::Buy;
}

OrderStatus BinanceWebSocket::String2OrderStatus(std::string s)
{
    if (s == "NEW")
        return OrderStatus::New;
    if (s == "FILLED")
        return OrderStatus::Filled;
    if (s == "CANCELED")
        return OrderStatus::Canceled;

    return OrderStatus::New;
}

OrderType BinanceWebSocket::String2OrderType(std::string s)
{
    if (s == "MARKET")
        return OrderType::Market;
    if (s == "LIMIT")
        return OrderType::Limit;
    if (s == "STOP")
        return OrderType::StopLoss;
    if (s == "STOP_MARKET")
        return OrderType::StopLossMarket;
    if (s == "TAKE_PROFIT")
        return OrderType::TakeProfit;
    if (s == "TAKE_PROFIT_MARKET")
        return OrderType::TakeProfitMarket;

    return OrderType::Limit;
}

MarginType BinanceWebSocket::String2MarginType(const std::string& type)
{
    if (type == "isolated")
        return MarginType::Isolated;
    return MarginType::Crossed;
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
    try {
        std::list<Position> Positions;
        timestamp_t UpdateTime = json.at("E").to_number<timestamp_t>();
        auto& P = json.at("p").as_array();
        for (auto& p : P) {
            Position position;
            position.Symbol = p.at("s").as_string().c_str();
            position.Side = String2Direct(p.at("ps").as_string().c_str());
            position.Qty = atof(p.at("pa").as_string().c_str());
            position.Type = String2MarginType(p.at("mt").as_string().c_str());
            position.MarkPrice = atof(p.at("mp").as_string().c_str());
            position.UnrealizedProfit = atof(p.at("up").as_string().c_str());
            position.UpdateTime = UpdateTime;
        }

        if (UpdatePositionCallback_ != nullptr)
            UpdatePositionCallback_(Context_, Exchange_, Symbol_, Positions);
    } catch (std::exception& e) {
        ErrorMessage((F("<BinanceWebSocket::ParseMarginCall> Error parse Margin call: %s") % e.what()).str());
    }
}

void BinanceWebSocket::ParseAccountUpdate(const json::value& json)
{
    try {
        Balances balances;
        timestamp_t updateTime = json.at("E").to_number<timestamp_t>();
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

        auto& P = obj.at("P").as_array();
        std::list<Position> Positions;
        for (auto& p : P) {
            Position position;
            position.Qty = atof(p.at("pa").as_string().c_str());
            position.EntryPrice = atof(p.at("ep").as_string().c_str());
            position.Symbol = p.at("s").as_string().c_str();
            position.Side = String2Direct(p.at("ps").as_string().c_str());
            position.UnrealizedProfit = atof(p.at("up").as_string().c_str());
            position.Type = String2MarginType(p.at("mt").as_string().c_str());
            position.UpdateTime = updateTime;
            Positions.push_back(position);
        }

        if (UpdatePositionCallback_ != nullptr)
            UpdatePositionCallback_(Context_, Exchange_, Symbol_, Positions);

    } catch (std::exception& e) {
        ErrorMessage((F("<BinanceWebSocket::ParseAccountUpdate> Error parse Account: %s") % e.what()).str());
    }
}

void BinanceWebSocket::ParseOrderTrade(const json::value& json)
{
    try {
        timestamp_t UpdateTime = json.at("E").to_number<timestamp_t>();
        auto& obj = json.at("o").as_object();
        Order order;
        order.Id = obj.at("i").to_number<unsigned long long>();
        order.UpdateTime = UpdateTime;
        order.Symbol = obj.at("s").as_string().c_str();
        order.Side = String2Direct(obj.at("S").as_string().c_str());
        order.Price = atof(obj.at("p").as_string().c_str());
        order.StopPrice = atof(obj.at("sp").as_string().c_str());
        order.Qty = atof(obj.at("q").as_string().c_str());
        order.Status = String2OrderStatus(obj.at("X").as_string().c_str());
        order.Time = obj.at("T").to_number<timestamp_t>();
        order.Type = String2OrderType(obj.at("o").as_string().c_str());
        if (UpdateOrderCallback_ != nullptr)
            UpdateOrderCallback_(Context_, Exchange_, Symbol_, order);

    } catch (std::exception& e) {
        ErrorMessage((F("<BinanceWebSocket::ParseOrderTrade> Error parse Order: %s") % e.what()).str());
    }
}

void BinanceWebSocket::ParseMarkPrice(const json::value& json)
{
    try {
        MarkPrice price;
        price.MarkPrice = atof(json.at("p").as_string().c_str());
        price.IndexPrice = atof(json.at("i").as_string().c_str());
        price.Symbol = json.at("s").as_string().c_str();
        price.Time = json.at("E").to_number<timestamp_t>();

        if (UpdateMarkPriceCallback_ != nullptr)
            UpdateMarkPriceCallback_(Context_, Exchange_, Symbol_, price);

    } catch (std::exception& e) {
        ErrorMessage((F("<BinanceWebSocket::ParseMarkPrice> Error parse Order: %s") % e.what()).str());
    }
}
