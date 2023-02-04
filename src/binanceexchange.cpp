#include "binanceexchange.h"

#include <boost/format.hpp>
#include <cstring>
#include <iostream>

#define F(S) boost::format(S)

#define JSON_PARSE \
    boost::json::stream_parser parser; \
    parser.write(str_result); \
    auto value = parser.release(); \
    if (IsError(value)) { \
        return -1; \
    } \

BinanceExchange::BinanceExchange()
{
    HOST = "api.binance.com";
    PORT = "443";
    API = "/api";
}

BinanceExchange::~BinanceExchange()
{

}

bool BinanceExchange::GetListenKey(std::string &listen_key)
{
    string str_result;
    string url = GetListenKeyEndpoint();
    PostUrl(url, str_result);
    bool ret = false;

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseListenKey(value, listen_key);
        } catch (std::exception& e) {
            ErrorMessage((F("<GetListenKey> Error ! %s") % e.what()).str());
        }
    } else
        ErrorMessage("<GetListenKey> Failed to get anything.");

    return ret;
}

bool BinanceExchange::PutListenKey(const std::string& key)
{
    string str_result;
    string url = API + "/v1/listenKey";
    PutUrl(url, str_result);
    bool ret = false;;

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = true;
        } catch (std::exception& e) {
            ErrorMessage((F("<PutListenKey> Error ! %s") % e.what()).str());
        }
    } else
        ErrorMessage("<PutListenKey> Failed to get anything.");

    return ret;
}

bool BinanceExchange::CloseListenKey(const std::string& key)
{
    string str_result;
    string url = API + "/v1/listenKey";
    DeleteUrl(url, str_result);
    bool ret = false;;

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = true;
        } catch (std::exception& e) {
            ErrorMessage((F("<CloseListenKey> Error ! %s") % e.what()).str());
        }
    } else
        ErrorMessage("<CloseListenKey> Failed to get anything.");

    return ret;
}

const string BinanceExchange::GetTimeEndpoint()
{
    return API + "/v1/time";
}

timestamp_t BinanceExchange::ParseServerTime(const json::value &json)
{
    return json.at("serverTime").to_number<timestamp_t>();
}

const string BinanceExchange::GetSymbolsEndpoint()
{
    return API + "/v1/exchangeInfo";
}

bool BinanceExchange::ParseSymbols(const boost::json::value& json, std::list<Symbol> &symbols)
{
    symbols.clear();
    try {
        for (auto& i : json.at("symbols").as_array()) {
            if (i.at("status") == "TRADING") {
                Symbol s(true);
                s.SetExchange("binance");
                s.Base.AssetPrecision = i.at("baseAssetPrecision").to_number<int>();
                s.Base.ComissionPrecision = i.at("baseCommissionPrecision").to_number<int>();
                s.Base.Name = i.at("baseAsset").as_string().c_str();

                s.Quote.AssetPrecision = i.at("quoteAssetPrecision").to_number<int>();
                s.Quote.ComissionPrecision = i.at("quoteCommissionPrecision").to_number<int>();
                s.Quote.Name = i.at("quoteAsset").as_string().c_str();

                if (i.at("filters").is_array()) {
                    auto& filters = i.at("filters").as_array();
                    for (auto& f : filters) {
                        auto type = f.at("filterType").as_string();
                        if (type == "PRICE_FILTER")
                            s.SetPriceStep(std::stod(f.at("tickSize").as_string().c_str()));
                        if (type == "LOT_SIZE")
                            s.SetQtyStep(std::stod(f.at("stepSize").as_string().c_str()));
                    }
                }
                symbols.push_back(s);
            }
        }
    } catch (std::exception& e) {
        ErrorMessage("Error get symbols !!!");
        return false;
    }

    return true;
}

const std::string BinanceExchange::GetListenKeyEndpoint()
{
    return API + "/v3/userDataStream";
}

bool BinanceExchange::ParseListenKey(const json::value& value, std::string& key)
{
    try {
        key = value.at("listenKey").as_string().c_str();
        return true;
    } catch (std::exception& e) {
        ErrorMessage(e.what());
        return false;
    }

    return false;
}

const string BinanceExchange::GetAccountInfoEndpoint(timestamp_t time)
{
    string query = "timestamp=" + std::to_string(time) +
            "&recvWindow=" + std::to_string(RecvWindow_);
    string signature =  hmac_sha256(SecretKey_.c_str(), query.c_str());
    return API + "/v3/account?" + query + "&signature=" + signature;
}

bool BinanceExchange::ParseAccountInfo(const boost::json::value& json, AccountInfo& info)
{
    info.Balance.clear();
    try {
        info.AccountType = json.at("accountType").as_string().c_str();
        for (auto& b : json.at("balances").as_array()) {
            Balance balance;
            balance.Asset = b.at("asset").as_string().c_str();
            balance.Free = std::stod(b.at("free").as_string().c_str());
            balance.Locked = std::stod(b.at("locked").as_string().c_str());
            info.Balance.push_back(balance);
        }
    } catch (std::exception& e) {
        ErrorMessage(e.what());
        return false;
    }

    return true;
}

const string BinanceExchange::GetTicker24Endpoint()
{
    return API + "/v3/ticker/24hr";
}

bool BinanceExchange::ParseTicker24(boost::json::value& value, std::list<Ticker24h>& tickers)
{
    tickers.clear();
    try {
        for (auto& p : value.as_array()) {
            Ticker24h ticker;
            ticker.Exchange = "binance";

            ticker.Symbol = p.at("symbol").as_string().c_str();
            ticker.High = std::stod(p.at("highPrice").as_string().c_str());
            ticker.LastPrice = std::stod(p.at("lastPrice").as_string().c_str());
            ticker.Low = std::stod(p.at("lowPrice").as_string().c_str());
            ticker.Open = std::stod(p.at("openPrice").as_string().c_str());
            ticker.QuoteVolume = std::stod(p.at("quoteVolume").as_string().c_str());
            ticker.Volume = std::stod(p.at("volume").as_string().c_str());

            tickers.push_back(ticker);
        }
    } catch (std::exception& e) {
        ErrorMessage(e.what());
        return false;
    }

    return true;
}

const string BinanceExchange::GetMarketDepthEndpoint(const std::string& symbol, int limit)
{
    return API + "/v1/depth?symbol=" + symbol + "&limit=" + std::to_string(limit);
}

bool BinanceExchange::ParseMarketDepth(const boost::json::value &json, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId)
{
    try {
        Asks.clear();
        Bids.clear();
        lastUpdateId = json.at("lastUpdateId").as_int64();
        auto& AsksList = json.at("asks").as_array();
        auto& BidsList = json.at("bids").as_array();

        for (auto& a : AsksList) {
            Depth d;
            d.Type = Depth::New;
            d.Price = std::stod(a.at(0).as_string().c_str());
            d.Qty = std::stod(a.at(1).as_string().c_str());
            Asks.push_back(d);
        }

        for (auto& b : BidsList) {
            Depth d;
            d.Type = Depth::New;
            d.Price = std::stod(b.at(0).as_string().c_str());
            d.Qty = std::stod(b.at(1).as_string().c_str());
            Bids.push_back(d);
        }
    } catch (std::exception& e) {
        ErrorMessage(e.what());
        return false;
    }

    return true;
}

const string BinanceExchange::GetTradesEndpoint(const std::string& symbol, timestamp_t start_time, timestamp_t end_time, int limit)
{
    std::string query = API + "/v1/aggTrades?";
    query += "symbol=" + symbol;
    if (start_time != 0 && end_time != 0) {
        query += "&startTime=" + std::to_string(start_time);
        query += "&endTime=" + std::to_string(end_time);
    }

    if (limit != 0) {
        query += "&limit=" + std::to_string(limit);
    }

    return query;
}

bool BinanceExchange::ParseTrades(boost::json::value& value, TradesList& trades)
{
    try {
        trades.clear();
        for (auto& t : value.as_array()) {
            Trade trade;
            trade.Id = t.at("a").to_number<uint64_t>();
            trade.Time = t.at("T").to_number<uint64_t>();
            trade.Price = std::stod(t.at("p").as_string().c_str());
            trade.Qty = std::stod(t.at("q").as_string().c_str());
            trade.IsBuy = !t.at("m").as_bool();
            trades.push_back(trade);
        }
    } catch (std::exception& e) {
        ErrorMessage(e.what());
        return false;
    }

    return true;
}

const string BinanceExchange::GetCandlesEndpoint(const std::string& symbol, TimeFrame tf, timestamp_t start, timestamp_t end, int limit)
{
    string query = API + "/v1/klines?";
    query += "symbol=" + symbol;
    query += "&interval=" + TimeFrame2String(tf);

    if (start != 0 && end != 0) {
        query += "&startTime=" + std::to_string(start);
        query += "&endTime=" + std::to_string(end);
    } else  {
        query += "&limit=" + std::to_string(limit);
    }

    return query;
}

bool BinanceExchange::ParseCandles(boost::json::value& value, CandlesList& candles)
{
    candles.clear();
    try {
        for (auto& c : value.as_array()) {
            Candle candle;
            candle.OpenTime = c.at(0).to_number<uint64_t>();
            candle.Open = std::stod(c.at(1).as_string().c_str());
            candle.High = std::stod(c.at(2).as_string().c_str());
            candle.Low = std::stod(c.at(3).as_string().c_str());
            candle.Close = std::stod(c.at(4).as_string().c_str());
            candle.Qty = std::stod(c.at(5).as_string().c_str());
            candle.CloseTime = c.at(6).to_number<uint64_t>();
            candle.NumberOfTrades = c.at(8).to_number<uint64_t>();
            candles.push_back(candle);
        }
    } catch (std::exception& e) {
        ErrorMessage(e.what());
        return false;
    }

    return true;
}

const string BinanceExchange::GetOpenOrdersEndpoint(timestamp_t time)
{
    string url = API + "/v1/openOrders?";
    string query = "timestamp=" + std::to_string(time);
    query += "&recvWindow=" + std::to_string(RecvWindow_);
    string signature =  hmac_sha256(SecretKey_.c_str(), query.c_str());
    return url + query + "&signature=" + signature;
}

bool BinanceExchange::ParseOrders(boost::json::value& value, OrderList& orders)
{
    orders.clear();
    try {
        for (auto& o : value.as_array()) {
            auto& order = o.as_object();
            Order neworder;
            neworder.Id = order.at("orderId").to_number<uint64_t>();
            neworder.StopPrice = std::atof(order.at("stopPrice").as_string().c_str());
            neworder.Price = std::atof(order.at("price").as_string().c_str());
            neworder.Qty = std::atof(order.at("origQty").as_string().c_str());
            neworder.Side = String2OrderSide(order.at("side").as_string().c_str());
            neworder.Status = String2OrderStatus(order.at("status").as_string().c_str());
            neworder.Symbol = order.at("symbol").as_string().c_str();
            neworder.Type = String2OrderType(order.at("type").as_string().c_str());
            neworder.Time = order.at("time").to_number<timestamp_t>();
            neworder.UpdateTime = order.at("updateTime").to_number<timestamp_t>();

            orders.push_back(neworder);
        }
    } catch (std::exception& exception) {
        ErrorMessage(exception.what());
        return false;
    }

    return true;
}

const string BinanceExchange::GetNewOrderEndpoint(timestamp_t time, const string& symbol, Direct direct, OrderType type, double qty, double price, double stopPrice)
{
    string query("timestamp=");
    query.append(std::to_string(time));
    query.append("&recvWindow=");
    query.append(std::to_string(RecvWindow_));
    query.append("&symbol=");
    query.append(symbol);

    query.append("&type=");
    query.append(OrderType2String(type));
    if (type == OrderType::StopLossMarket || type == OrderType::TakeProfitMarket)
        query.append("&closePosition=true");

    query.append("&side=");
    query.append(OrderSide2String(direct));

    query.append("&quantity=");
    query.append(std::to_string(qty));

    if (type == OrderType::Limit || type == OrderType::StopLoss || type == OrderType::TakeProfit) {
        query.append("&price=");
        query.append(std::to_string(price));
        query.append("&timeInForce=");
        query.append("GTC");
    }

    if (type == OrderType::StopLoss || type == OrderType::TakeProfit ||
        type == OrderType::StopLossMarket || type == OrderType::TakeProfitMarket) {
        query.append("&stopPrice=");
        query.append(std::to_string(stopPrice));
    }

    string signature =  hmac_sha256(SecretKey_.c_str(), query.c_str());
    query.append("&signature=");
    query.append(signature);

    return API + "/v1/order?" + query;
}

bool BinanceExchange::ParseNewOrder(boost::json::value& value, Order& order)
{
    try {
        order.Id = value.at("orderId").to_number<uint64_t>();
        order.Price = atof(value.at("price").as_string().c_str());
        order.StopPrice = atof(value.at("stopPrice").as_string().c_str());
        order.Qty = atof(value.at("origQty").as_string().c_str());
        order.Side = String2OrderSide(value.at("side").as_string().c_str());
        order.Symbol = value.at("symbol").as_string().c_str();
        order.Status = String2OrderStatus(value.at("status").as_string().c_str());
        order.Time = value.at("updateTime").to_number<timestamp_t>();
        order.UpdateTime = value.at("updateTime").to_number<timestamp_t>();
        order.Type = String2OrderType(value.at("type").as_string().c_str());
    } catch (std::exception& e) {
        ErrorMessage(e.what());
        return false;
    }

    return true;
}

const string BinanceExchange::GetCancelOrderEndpoint(timestamp_t time, Order& order)
{
    string query("timestamp=");
    query.append(std::to_string(time));
    query.append("&recvWindow=");
    query.append(std::to_string(RecvWindow_));
    query.append("&symbol=");
    query.append(order.Symbol);

    query.append("&orderId=");
    query.append(std::to_string(order.Id));

    string signature =  hmac_sha256(SecretKey_.c_str(), query.c_str());
    query.append("&signature=");
    query.append(signature);

    return API + "/v1/order?" + query;
}

bool BinanceExchange::ParseCancelOrder(boost::json::value& value)
{
    try {
        if (value.at("code").to_number<int>() < 0)
            return false;
    } catch (std::exception& e) {
        return true;
    }

    return true;
}

Direct BinanceExchange::String2OrderSide(std::string s)
{
    if (s == "BUY")
        return Direct::Buy;

    if (s == "SELL")
        return Direct::Sell;

    if (s == "BOTH")
        return Direct::Both;

    return Direct::Buy;
}

OrderStatus BinanceExchange::String2OrderStatus(std::string s)
{
    if (s == "NEW")
        return OrderStatus::New;
    if (s == "FILLED")
        return OrderStatus::Filled;
    if (s == "CANCELED")
        return OrderStatus::Canceled;

    return OrderStatus::New;
}

OrderType BinanceExchange::String2OrderType(std::string s)
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

string BinanceExchange::OrderSide2String(Direct direct)
{
    switch (direct) {
    case Direct::Buy:
        return "BUY";
    case Direct::Sell:
        return "SELL";
    case Direct::Both:
        return "BOTH";
    }

    return "BUY";
}

string BinanceExchange::OrderType2String(OrderType type)
{
    switch (type) {
    case OrderType::Limit:
        return "LIMIT";
    case OrderType::Market:
        return "MARKET";
    case OrderType::StopLoss:
        return "STOP";
    case OrderType::StopLossMarket:
        return "STOP_MARKET";
    case OrderType::TakeProfitMarket:
        return "TAKE_PROFIT_MARKET";
    case OrderType::TakeProfit:
        return "TAKE_PROFIT";
    }

    return "LIMIT";
}
