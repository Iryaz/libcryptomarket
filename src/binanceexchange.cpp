#include "binanceexchange.h"

#include <cstring>
#include <hmac.h>
#include <sha.h>
#include <iostream>

BinanceExchange::BinanceExchange()
{
    ApiType_ = API_PATH;
    ApiServer_ = BINANCE_SERVER;
}

BinanceExchange::~BinanceExchange()
{

}

string BinanceExchange::hmac_sha256(const char *key, const char *data)
{
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), key, strlen(key), (unsigned char*)data, strlen(data), nullptr, nullptr);
    return b2a_hex((char *)digest, 32);
}

string BinanceExchange::b2a_hex(char *byte_arr, int n)
{
    const static std::string HexCodes = "0123456789abcdef";
    string HexString;
    for (int i = 0; i < n ; ++i) {
        unsigned char BinValue = byte_arr[i];
        HexString += HexCodes[( BinValue >> 4 ) & 0x0F];
        HexString += HexCodes[BinValue & 0x0F];
    }

    return HexString;
}

timestamp_t BinanceExchange::ParseServerTime(const json::value &json)
{
    return json.at("serverTime").as_uint64();
}

string BinanceExchange::BuildTimeUrl()
{
    return ApiServer_ + ApiType_ + "/v1/time";
}

string BinanceExchange::BuildExchangeInfoUrl()
{
    return ApiServer_ + ApiType_ + "/v1/exchangeInfo";
}

string BinanceExchange::BuildAllPricesUrl()
{
    return ApiServer_ + ApiType_ + "/v1/ticker/allPrices";
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

bool BinanceExchange::ParseExchangeInfo(const json::value &json, ExchangeInfo &info)
{
    info.Symbols.clear();
    for (auto i : json.at("symbols").as_array()) {
        if (i.at("status") == "TRADING") {
            Symbol s(true);
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

            info.Symbols.push_back(s);
        }
    }

    return true;
}

bool BinanceExchange::ParseAllPrices(const json::value &json, Prices& prices)
{
    prices.clear();
    for (auto p : json.as_array()) {
        Price price;
        price.symbol = p.at("symbol").as_string().c_str();
        price.price = std::stod(p.at("price").as_string().c_str());
        prices.push_back(price);
    }
    return true;
}

bool BinanceExchange::ParseMarketDepth(const json::value &json, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId)
{
    Asks.clear();
    Bids.clear();
    lastUpdateId = json.at("lastUpdateId").as_int64();
    auto AsksList = json.at("asks").as_array();
    auto BidsList = json.at("bids").as_array();

    for (auto a : AsksList) {
        Depth d;
        d.Price = std::stod(a.at(0).as_string().c_str());
        d.Qty = std::stod(a.at(1).as_string().c_str());
        Asks.push_back(d);
    }

    for (auto b : BidsList) {
        Depth d;
        d.Price = std::stod(b.at(0).as_string().c_str());
        d.Qty = std::stod(b.at(1).as_string().c_str());
        Bids.push_back(d);
    }

    return true;
}

bool BinanceExchange::ParseAggregateTradesList(const json::value &json, TradesList& trades)
{
    trades.clear();
    for (auto t : json.as_array()) {
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
    for (auto c : json.as_array()) {
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
    for (auto b : json.at("balances").as_array()) {
        Balance balance;
        balance.Asset = b.at("asset").as_string().c_str();
        balance.Free = std::stod(b.at("free").as_string().c_str());
        balance.Locked = std::stod(b.at("locked").as_string().c_str());
        info.Balance.push_back(balance);
    }

    return true;
}
