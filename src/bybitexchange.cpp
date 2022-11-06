#include "bybitexchange.h"
#include <iostream>

BybitExchange::BybitExchange()
{
    ApiServer_ = BYBIT_SERVER;
}

string BybitExchange::BuildTimeUrl()
{
    return ApiServer_ + "/spot/v1/time";
}

string BybitExchange::BuildSymbolsUrl()
{
    return ApiServer_ + "/spot/v1/symbols";
}

string BybitExchange::BuildTicker24Url()
{
    return ApiServer_ + "/spot/v3/public/quote/ticker/24hr";
}

string BybitExchange::BuildMarketDepthUrl(const string symbol, int limit)
{
    return ApiServer_ + "/spot/quote/v1/depth?symbol=" + symbol + "&limit=" + std::to_string(limit);
}

string BybitExchange::BuildAggregateTradesUrl(const string symbol, timestamp_t start_time, timestamp_t end_time, int limit)
{
    return ApiServer_ + "/spot/quote/v1/trades?symbol=" + symbol + "&limit=" + std::to_string(limit);
}

string BybitExchange::BuildCandlesUrl(const string symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit)
{
    std::string url;
    url += ApiServer_ + "/spot/quote/v1/kline?symbol=" + symbol + "&interval=" +
            GetInterval(tf);

    if (start_time != 0)
        url += "&startTime=" + std::to_string(start_time);

    if (end_time != 0)
        url += "&endTime=" + std::to_string(end_time);

    if (limit != 0)
        url += "&limit=" + std::to_string(limit);

    return url;
}

string BybitExchange::BuildAccountUrl(timestamp_t timestamp)
{
    return ApiServer_ + "/spot/v1/account";
}

timestamp_t BybitExchange::ParseServerTime(const json::value& value)
{
    try {
        if (value.at("ret_code").to_number<int>() == 0)
            return value.at("result").as_object().at("serverTime").to_number<timestamp_t>();
        else
            return -1;
    } catch (std::exception& e) {
        ErrorMessage("<BybitExchange::ParseServerTime> Error parse json !!!");
        return -1;
    }
}

bool BybitExchange::ParseSymbols(const json::value& value, std::list<Symbol> &symbols)
{
    try {
        symbols.clear();
        if (value.at("ret_code").to_number<int>() != 0)
            return false;

        auto& symbols_list = value.at("result").as_array();
        for (auto& i : symbols_list) {
            auto& s = i.as_object();
            Symbol symbol(true);
            symbol.SetExchange("bybit");
            symbol.Base.Name = s.at("baseCurrency").as_string().c_str();
            symbol.Base.AssetPrecision = std::atof(s.at("basePrecision").as_string().c_str());
            symbol.Quote.Name = s.at("quoteCurrency").as_string().c_str();
            symbol.QuotePrecison =  std::atof(s.at("quotePrecision").as_string().c_str());
            symbol.SetPriceStep(std::atof(s.at("minPricePrecision").as_string().c_str()));
            symbol.SetQtyStep(std::atof(s.at("basePrecision").as_string().c_str()));
            symbols.push_back(symbol);
        }
    } catch (std::exception& e) {
        ErrorMessage("<BybitExchange::ParseSymbols> Error Parse Json");
        return false;
    }

    return true;
}

bool BybitExchange::ParseTicker24(const json::value& value, std::list<Ticker24h>& tickers)
{
    try {
        tickers.clear();
        if (value.at("ret_code").to_number<int>() != 0)
            return false;

        auto& ticker_array = value.at("result").as_array();
        for (auto& i : ticker_array) {
            Ticker24h ticker;
            auto& p = i.as_object();
            ticker.Exchange = "bybit";
            ticker.Symbol = p.at("s").as_string();
            ticker.High = std::atof(p.at("h").as_string().c_str());
            ticker.Low = std::atof(p.at("l").as_string().c_str());
            ticker.Open = std::atof(p.at("o").as_string().c_str());
            ticker.LastPrice = std::atof(p.at("lp").as_string().c_str());
            ticker.QuoteVolume = std::atof(p.at("qv").as_string().c_str());
            ticker.Volume = std::atof(p.at("v").as_string().c_str());

            tickers.push_back(ticker);
        }
        return true;
    } catch (std::exception& e) {
        ErrorMessage("<BybitExchange::ParseTicker24> Error Parse Json");
        return false;
    }

    return true;
}

bool BybitExchange::ParseMarketDepth(const json::value& value, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId)
{
    try {
        Asks.clear();
        Bids.clear();
        if (value.at("ret_code").to_number<int>() != 0)
            return false;

        auto& bids_array = value.at("result").at("bids").as_array();
        for (auto& i : bids_array) {
            Depth depth;
            depth.Type = Depth::New;
            auto d = i.as_array();
            depth.Price = std::atof(d.at(0).as_string().c_str());
            depth.Qty = std::atof(d.at(1).as_string().c_str());
            Bids.push_back(depth);
        }

        auto& asks_array = value.at("result").at("asks").as_array();
        for (auto& i : asks_array) {
            Depth depth;
            depth.Type = Depth::New;
            auto d = i.as_array();
            depth.Price = std::atof(d.at(0).as_string().c_str());
            depth.Qty = std::atof(d.at(1).as_string().c_str());
            Asks.push_back(depth);
        }

    } catch (std::exception& e) {
        ErrorMessage("<BybitExchange::ParseMarketDepth> Error Parse Json");
        return false;
    }

    return true;
}

bool BybitExchange::ParseAggregateTradesList(const json::value& value, TradesList& trades)
{
    try {
        trades.clear();
        if (value.at("ret_code").to_number<int>() != 0)
            return false;
        auto& array = value.at("result").as_array();
        for (auto& i : array) {
            auto& t = i.as_object();
            Trade trade;
            trade.IsBuy = t.at("isBuyerMaker").as_bool();
            trade.Time = t.at("time").to_number<timestamp_t>();
            trade.Id = -1;
            trade.Price = std::atof(t.at("price").as_string().c_str());
            trade.Qty = std::atof(t.at("qty").as_string().c_str());

            trades.push_back(trade);
        }

    } catch (std::exception& e) {
        ErrorMessage("<BybitExchange::ParseAggregateTradesLis> Error Parse Json");
        return false;
    }

    return true;
}

bool BybitExchange::ParseCandles(const json::value& value, CandlesList& candles)
{
    try {
        candles.clear();
        int code = value.at("ret_code").to_number<int>();
        if (code != 0)
            return false;
        auto& array = value.at("result").as_array();
        for (auto& i : array) {
            Candle candle;
            auto c = i.as_array();
            candle.OpenTime = c.at(0).to_number<timestamp_t>();
            candle.Open = std::atof(c.at(1).as_string().c_str());
            candle.High = std::atof(c.at(2).as_string().c_str());
            candle.Low = std::atof(c.at(3).as_string().c_str());
            candle.Close = std::atof(c.at(4).as_string().c_str());
            candle.Qty = std::atof(c.at(5).as_string().c_str());
            candle.CloseTime = c.at(6).to_number<timestamp_t>();
            candle.NumberOfTrades = c.at(8).as_int64();

            candles.push_back(candle);
        }
    } catch (std::exception& e) {
        ErrorMessage("<BybitExchange::ParseCandles> Error Parse Json");
        return false;
    }

    return true;
}

bool BybitExchange::ParseAccount(const json::value& value, AccountInfo& info)
{

}

const string BybitExchange::GetInterval(TimeFrame tf)
{
    switch (tf) {
    case libcryptomarket::TimeFrame_1m:
        return "1m";
        break;
    case libcryptomarket::TimeFrame_5m:
        return "5m";
        break;
    case libcryptomarket::TimeFrame_15m:
        return "15m";
        break;
    case libcryptomarket::TimeFrame_1h:
        return "1h";
        break;
    case libcryptomarket::TimeFrame_4h:
        return "4h";
        break;
    case libcryptomarket::TimeFrame_1d:
        return "1d";
        break;
    default:
        break;
    }

    return "";
}
