#include "bybitexchange.h"

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

string BybitExchange::BuildAllPricesUrl()
{
    return ApiServer_ + "/spot/v1/tickers";
}

string BybitExchange::BuildMarketDepthUrl(const string symbol, int limit)
{
    return ApiServer_ + "/spot/v1/orderBook/L2?symbol=" + symbol;
}

string BybitExchange::BuildAggregateTradesUrl(const string symbol, timestamp_t start_time, timestamp_t end_time, int limit)
{
    return ApiServer_ + "/v2/public/trading-records?symbol=" + symbol + "&limit=" + std::to_string(limit);
}

string BybitExchange::BuildCandlesUrl(const string symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit)
{
    return ApiServer_ + "/v2/public/kline/list?symbol=" + symbol + "&interval=" +
            GetInterval(tf) + "&from=" +
            std::to_string(start_time) + "&limit=" + std::to_string(limit);
}

string BybitExchange::BuildAccountUrl(timestamp_t timestamp)
{
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
        auto symbols_list = value.at("result").as_array();
        for (auto& i : symbols_list) {
            auto& s = i.as_object();
            Symbol symbol(true);
            symbol.Base.Name = s.at("baseCurrency").as_string().c_str();
            symbol.Base.AssetPrecision = std::atof(s.at("basePrecision").as_string().c_str());
            symbol.Quote.Name = s.at("quoteCurrency").as_string().c_str();
            symbol.QuotePrecison =  std::atof(s.at("quotePrecision").as_string().c_str());
            symbol.SetPriceStep(std::atof(s.at("minPricePrecision").as_string().c_str()));
            symbol.SetQtyStep(std::atof(s.at("basePrecision").as_string().c_str()));
            symbols.push_back(symbol);
        }
    } catch (std::exception& e) {
        return false;
    }

    return true;
}

bool BybitExchange::ParseAllPrices(const json::value& value, Prices& prices)
{
}

bool BybitExchange::ParseMarketDepth(const json::value& value, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId)
{
}

bool BybitExchange::ParseAggregateTradesList(const json::value& value, TradesList& trades)
{
}

bool BybitExchange::ParseCandles(const json::value& value, CandlesList& candles)
{
}

bool BybitExchange::ParseAccount(const json::value& value, AccountInfo& info)
{

}

const string BybitExchange::GetInterval(TimeFrame tf)
{
    switch (tf) {
    case libcryptomarket::TimeFrame_1m:
        return "1";
        break;
    case libcryptomarket::TimeFrame_5m:
        return "5";
        break;
    case libcryptomarket::TimeFrame_15m:
        return "15";
        break;
    case libcryptomarket::TimeFrame_1h:
        return "60";
        break;
    case libcryptomarket::TimeFrame_4h:
        return "240";
        break;
    case libcryptomarket::TimeFrame_1d:
        return "720";
        break;
    default:
        break;
    }

    return "";
}
