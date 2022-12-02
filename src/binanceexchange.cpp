#include "binanceexchange.h"

#include <cstring>
#include <iostream>

BinanceExchange::BinanceExchange()
{
    ApiType_ = API_PATH;
    ApiServer_ = BINANCE_SERVER;
}

BinanceExchange::~BinanceExchange()
{

}

timestamp_t BinanceExchange::ParseServerTime(const json::value &json)
{
    return json.at("serverTime").to_number<timestamp_t>();
}

string BinanceExchange::BuildTimeUrl()
{
    return ApiServer_ + ApiType_ + "/v1/time";
}

string BinanceExchange::BuildSymbolsUrl()
{
    return ApiServer_ + ApiType_ + "/v1/exchangeInfo";
}

string BinanceExchange::BuildTicker24Url()
{
    return ApiServer_ + ApiType_ + "/v3/ticker/24hr";
}

string BinanceExchange::BuildMarketDepthUrl(const string symbol, int limit)
{
    string url = ApiServer_;
    url += ApiType_ + "/v1/depth?";

    string querystring("symbol=");
    querystring.append(symbol);
    querystring.append("&limit=");
    querystring.append(std::to_string(limit));

    url.append(querystring);

    return url;
}

string BinanceExchange::BuildAggregateTradesUrl(const string symbol, timestamp_t start_time, timestamp_t end_time, int limit)
{
    string url = ApiServer_;
    url += ApiType_ + "/v1/aggTrades?";

    string querystring("symbol=");
    querystring.append(symbol);

    if (start_time != 0 && end_time != 0) {
        querystring.append("&startTime=");
        querystring.append(std::to_string(start_time));
        querystring.append("&endTime=");
        querystring.append(std::to_string(end_time));
    } else  {
        querystring.append("&limit=");
        querystring.append(std::to_string(limit));
    }

    url.append(querystring);

    return url;
}

string BinanceExchange::BuildCandlesUrl(const string symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit)
{
    string url = ApiServer_;
    url += ApiType_ + "/v1/klines?";

    string querystring("symbol=");
    querystring.append(symbol);

    querystring.append("&interval=");
    querystring.append(Timeframe2String(tf));

    if (start_time != 0 && end_time != 0) {
        querystring.append("&startTime=");
        querystring.append(std::to_string(start_time));
        querystring.append("&endTime=");
        querystring.append(std::to_string(end_time));
    } else  {
        querystring.append("&limit=");
        querystring.append(std::to_string(limit));
    }

    url.append(querystring);

    return url;
}

string BinanceExchange::BuildAccountUrl(timestamp_t timestamp)
{
    string url = ApiServer_;
    url += ApiType_ + "/v3/account?";

    string querystring("timestamp=");
    querystring.append(std::to_string(timestamp));

    if (RecvWindow_ > 0) {
        querystring.append("&recvWindow=");
        querystring.append(std::to_string(RecvWindow_));
    }

    string signature =  hmac_sha256(SecretKey_.c_str(), querystring.c_str());
    querystring.append("&signature=");
    querystring.append(signature);

    url.append(querystring);

    return url;
}

string BinanceExchange::BuildOpenOrdersUrl(timestamp_t timestamp)
{
    string url = ApiServer_;
    url += ApiType_ + "/v1/openOrders?";

    string querystring("timestamp=");
    querystring.append(std::to_string(timestamp));

    if (RecvWindow_ > 0) {
        querystring.append("&recvWindow=");
        querystring.append(std::to_string(RecvWindow_));
    }

    string signature =  hmac_sha256(SecretKey_.c_str(), querystring.c_str());
    querystring.append("&signature=");
    querystring.append(signature);

    url.append(querystring);

    return url;
}

bool BinanceExchange::ParseSymbols(const json::value &json, std::list<Symbol> &symbols)
{
    symbols.clear();
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
                s.SetPriceStep(std::stod(i.at("filters").at(0).at("tickSize").as_string().c_str()));
                s.SetQtyStep(std::stod(i.at("filters").at(2).at("stepSize").as_string().c_str()));
            }

            symbols.push_back(s);
        }
    }

    return true;
}

bool BinanceExchange::ParseTicker24(const json::value& value, std::list<Ticker24h>& tickers)
{
    tickers.clear();
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

    return true;
}

bool BinanceExchange::ParseMarketDepth(const json::value &json, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId)
{
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

    return true;
}

bool BinanceExchange::ParseAggregateTradesList(const json::value &json, TradesList& trades)
{
    trades.clear();
    for (auto& t : json.as_array()) {
        Trade trade;
        trade.Id = t.at("a").to_number<uint64_t>();
        trade.Time = t.at("T").to_number<uint64_t>();
        trade.Price = std::stod(t.at("p").as_string().c_str());
        trade.Qty = std::stod(t.at("q").as_string().c_str());
        trade.IsBuy = !t.at("m").as_bool();
        trades.push_back(trade);
    }

    return true;
}

bool BinanceExchange::ParseCandles(const json::value &json, CandlesList& candles)
{
    candles.clear();
    for (auto& c : json.as_array()) {
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

    return true;
}

bool BinanceExchange::ParseAccount(const json::value &json, AccountInfo &info)
{
    info.Balance.clear();
    info.AccountType = json.at("accountType").as_string().c_str();
    for (auto& b : json.at("balances").as_array()) {
        Balance balance;
        balance.Asset = b.at("asset").as_string().c_str();
        balance.Free = std::stod(b.at("free").as_string().c_str());
        balance.Locked = std::stod(b.at("locked").as_string().c_str());
        info.Balance.push_back(balance);
    }

    return true;
}

/*
 * [
  {
    "avgPrice": "0.00000",
    "clientOrderId": "abc",
    "cumQuote": "0",
    "executedQty": "0",
    "orderId": 1917641,
    "origQty": "0.40",
    "origType": "TRAILING_STOP_MARKET",
    "price": "0",
    "reduceOnly": false,
    "side": "BUY",
    "positionSide": "SHORT",
    "status": "NEW",
    "stopPrice": "9300",                // please ignore when order type is TRAILING_STOP_MARKET
    "closePosition": false,   // if Close-All
    "symbol": "BTCUSDT",
    "time": 1579276756075,              // order time
    "timeInForce": "GTC",
    "type": "TRAILING_STOP_MARKET",
    "activatePrice": "9020",            // activation price, only return with TRAILING_STOP_MARKET order
    "priceRate": "0.3",                 // callback rate, only return with TRAILING_STOP_MARKET order
    "updateTime": 1579276756075,        // update time
    "workingType": "CONTRACT_PRICE",
    "priceProtect": false            // if conditional order trigger is protected
  }
]
 * */

bool BinanceExchange::ParseOpenOrders(const json::value& value, OrderList& orders)
{
    orders.clear();
    for (auto& o : value.as_array()) {
        auto& order = o.as_object();
        Order neworder;
        neworder.Id = order.at("orderId").to_number<uint64_t>();
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

    return true;
}

Direct BinanceExchange::String2OrderSide(std::string s)
{
    if (s == "BUY")
        return Direct::Buy;

    if (s == "SELL")
        return Direct::Sell;

    return Direct::Buy;
}

OrderStatus BinanceExchange::String2OrderStatus(std::string s)
{
    if (s == "NEW")
        return OrderStatus::New;
    if (s == "FILLED")
        return OrderStatus::Filled;
    if (s == "CANCELLED")
        return OrderStatus::Cancelled;

    return OrderStatus::New;
}

OrderType BinanceExchange::String2OrderType(std::string s)
{
    if (s == "TRAILING_STOP_MARKET")
        return OrderType::StopMarket;
    if (s == "MARKET")
        return OrderType::Market;
    if (s == "LIMIT")
        return OrderType::Limit;
    if (s == "STOP/TAKE_PROFIT")
        return OrderType::TakeProfit;
    if (s == "STOP_MARKET/TAKE_PROFIT_MARKET")
        return OrderType::TakeProfitMarket;

    return OrderType::Limit;
}
