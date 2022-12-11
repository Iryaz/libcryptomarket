#include "websocket/bybitwebsocket.h"

#include <boost/beast/core.hpp>
#include <boost/asio.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/signals2.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>
#include <regex>

using std::cout;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

ByBitWebsocket::ByBitWebsocket(Type type, const std::string &symbol, int subscribe_flags) :
    BaseWebSocket(type, symbol, subscribe_flags)
{
    SetSymbol(symbol);

    switch (Type_) {
    case Spot: {
        SetHost("stream.bybit.com");
        SetPath("/spot/ws");
        SetWebSocketPort(443);
        Exchange_ = "bybit";
    } break;
    case Futures: {
        SetHost("stream.bytick.com");
        SetPath("/realtime_public");
        SetWebSocketPort(443);
        Exchange_ = "bybit-futures";
    } break;
    default:
        break;
    }

    Init(GetSubscribeFlags());
}

void ByBitWebsocket::SetSymbol(const std::string& symbol)
{
    Symbol_ = symbol;
    std::transform(Symbol_.begin(), Symbol_.end(), Symbol_.begin(), [](unsigned char c) {return std::toupper(c);});
}

void ByBitWebsocket::Init(int flag, const std::string& listen_key)
{
    boost::property_tree::ptree pt;
    boost::property_tree::ptree args;

    pt.put("op", "subscribe");
    if (flag & MARKET_DEPTH_SUBSCRIBE) {
        boost::property_tree::ptree arg;
        arg.put("", "orderBookL2_25." + Symbol_);
        args.push_back(std::make_pair("", arg));
    }

    if (flag & TRADES_SUBSCRIBE) {
        boost::property_tree::ptree arg;
        arg.put("", "trade." + Symbol_);
        args.push_back(std::make_pair("", arg));
    }

    if (flag & CANDLES_SUBSCRIBE_1m) {
        boost::property_tree::ptree arg;
        arg.put("", "candle.1." + Symbol_);
        args.push_back(std::make_pair("", arg));
    }

    if (flag & CANDLES_SUBSCRIBE_5m) {
        boost::property_tree::ptree arg;
        arg.put("", "candle.5." + Symbol_);
        args.push_back(std::make_pair("", arg));
    }

    if (flag & CANDLES_SUBSCRIBE_15m) {
        boost::property_tree::ptree arg;
        arg.put("", "candle.15." + Symbol_);
        args.push_back(std::make_pair("", arg));
    }

    if (flag & CANDLES_SUBSCRIBE_1h) {
        boost::property_tree::ptree arg;
        arg.put("", "candle.60." + Symbol_);
        args.push_back(std::make_pair("", arg));
    }

    if (flag & CANDLES_SUBSCRIBE_4h) {
        boost::property_tree::ptree arg;
        arg.put("", "candle.240." + Symbol_);
        args.push_back(std::make_pair("", arg));
    }

    if (flag & CANDLES_SUBSCRIBE_1d) {
        boost::property_tree::ptree arg;
        arg.put("", "candle.720." + Symbol_);
        args.push_back(std::make_pair("", arg));
    }

    pt.add_child("args", args);
    std::stringstream ss;
    boost::property_tree::json_parser::write_json(ss, pt);
    Message_ = ss.str();
}

void ByBitWebsocket::ParseJSon(boost::json::value& result)
{
    boost::json::value data;
    if (IsResponseMessage(result) == true)
        return;

    DataEventType event = String2EventType(result.at("topic").as_string().c_str());
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
}

void ByBitWebsocket::ParseMarketDepth(const json::value& json)
{
    MarketDepth buyseries;
    MarketDepth sellseries;
    auto data = json.at("data").as_object();
    timestamp_t time = std::stoull(json.at("timestamp_e6").as_string().c_str());

    if (json.at("type") == "snapshot") {
        auto& order_book = data.at("order_book").as_array();
        for (auto& i : order_book) {
            Depth d;
            d.Type = Depth::New;
            d.Price = atof(i.at("price").as_string().c_str());
            d.Qty = i.at("size").to_number<double>();
            if (i.at("side") == "Buy")
                buyseries.push_back(d);
            else
                sellseries.push_back(d);
        }
    }

    if (json.at("type") == "delta") {
        if (data.at("delete").is_array()) {
            auto& array = data.at("delete").as_array();
            for (auto& i : array) {
                Depth d;
                d.Type = Depth::Remove;
                d.Price = atof(i.at("price").as_string().c_str());
                d.Qty = i.at("size").to_number<double>();
                if (i.at("side") == "Buy")
                    buyseries.push_back(d);
                else
                    sellseries.push_back(d);
            }
        }
        if (data.at("insert").is_array()) {
            auto& array = data.at("insert").as_array();
            for (auto& i : array) {
                Depth d;
                d.Type = Depth::New;
                d.Price = atof(i.at("price").as_string().c_str());
                d.Qty = i.at("size").to_number<double>();
                if (i.at("side") == "Buy")
                    buyseries.push_back(d);
                else
                    sellseries.push_back(d);
            }
        }

        if (data.at("update").is_array()) {
            auto& array = data.at("update").as_array();
            for (auto& i : array) {
                Depth d;
                d.Type = Depth::Update;
                d.Price = atof(i.at("price").as_string().c_str());
                d.Qty = i.at("size").to_number<double>();
                if (i.at("side") == "Buy")
                    buyseries.push_back(d);
                else
                    sellseries.push_back(d);
            }
        }

    }

    if (UpdateMarketDepthCallback_ != nullptr)
        UpdateMarketDepthCallback_(Context_, Exchange_, Symbol_, sellseries, buyseries);
}

void ByBitWebsocket::ParseTrades(const json::value& json)
{
    Trade trade;
    auto& arr = json.at("data").as_array();
    for (const auto &itr : arr) {
        auto& obj = itr.as_object();
        auto& symbol = obj.at("symbol").as_string();
        trade.Price = atof(obj.at("price").as_string().c_str());
        trade.Qty = obj.at("size").as_double();
        if (obj.at("side").as_string() == "Buy")
            trade.IsBuy = true;
        else
            trade.IsBuy = false;
        trade.Time = std::stoull(obj.at("trade_time_ms").as_string().c_str());

        if (AddTradeCallback_ != nullptr)
            AddTradeCallback_(Context_, Exchange_, symbol.c_str(), trade);
    }
}

void ByBitWebsocket::ParseKLines(const json::value& json)
{
    try {
        auto& data = json.at("data").as_array();
        for (auto& itr : data) {
            auto& obj = itr.as_object();
            Candle c;
            c.OpenTime = obj.at("start").to_number<timestamp_t>();
            c.CloseTime = obj.at("end").to_number<timestamp_t>();
            c.Qty = std::atof(obj.at("volume").as_string().c_str());
            c.Open = obj.at("open").to_number<double>();
            c.Close = obj.at("close").to_number<double>();
            c.High = obj.at("high").to_number<double>();
            c.Low = obj.at("low").to_number<double>();

            int period = std::atoi(obj.at("period").as_string().c_str());
            TimeFrame tf = GetTimeFrameFromPeriod(period);
            if (UpdateCandleCallback_ != nullptr)
                UpdateCandleCallback_(Context_, Exchange_, Symbol_, tf, c);

        }
    } catch (std::exception& e) {
        ErrorMessage("<ByBitWebsocket::ParseKLines> Error parse KLines");
    }
}

bool ByBitWebsocket::IsResponseMessage(const json::value& result)
{
    try {
        bool is_success = result.at("success").as_bool();
        if (is_success == false)
            WarningMessage("<ByBitWebsocket::IsResponseMessage> Response Message not success !!!");

        return true;
    } catch (std::exception& e) {
        return false;
    }
}

BaseWebSocket::DataEventType ByBitWebsocket::String2EventType(const std::string& s)
{
    if (regex_match(s, std::regex("trade\.*")))
        return BaseWebSocket::DataEventType::AGG_TRADE;

    if (regex_match(s, std::regex("orderBookL2_25\.*")))
        return BaseWebSocket::DataEventType::DEPTH_UPDATE;

    if (regex_match(s, std::regex("candle\.1\.*")))
        return BaseWebSocket::DataEventType::KLINE;

    if (regex_match(s, std::regex("candle\.5\.*")))
        return BaseWebSocket::DataEventType::KLINE;

    if (regex_match(s, std::regex("candle\.15\.*")))
        return BaseWebSocket::DataEventType::KLINE;

    if (regex_match(s, std::regex("candle\.60\.*")))
        return BaseWebSocket::DataEventType::KLINE;

    if (regex_match(s, std::regex("candle\.240\.*")))
        return BaseWebSocket::DataEventType::KLINE;

    if (regex_match(s, std::regex("candle\.720\.*")))
        return BaseWebSocket::DataEventType::KLINE;

    return BaseWebSocket::DataEventType::UNKNOWN;
}

TimeFrame ByBitWebsocket::GetTimeFrameFromPeriod(int period)
{
    if (period == 1)
        return TimeFrame::TimeFrame_1m;

    if (period == 5)
        return TimeFrame::TimeFrame_5m;

    if (period == 15)
        return TimeFrame::TimeFrame_15m;

    if (period == 60)
        return TimeFrame::TimeFrame_1h;

    if (period == 240)
        return TimeFrame::TimeFrame_4h;

    if (period == 720)
        return TimeFrame::TimeFrame_1d;

    return TimeFrame::TimeFrame_1m;
}
