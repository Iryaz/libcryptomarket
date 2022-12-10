
#include "bybitfuturesexchange.h"

BybitFuturesExchange::BybitFuturesExchange()
{

}

string BybitFuturesExchange::GetListenKeyUrl()
{
    return "";
}

string BybitFuturesExchange::BuildTimeUrl()
{
    return ApiServer_ + "/v3/public/time";
}

string BybitFuturesExchange::BuildSymbolsUrl()
{
    return ApiServer_ + "/derivatives/v3/public/instruments-info";
}

string BybitFuturesExchange::BuildTicker24Url()
{
    return ApiServer_ + "/v2/public/tickers";
}

string BybitFuturesExchange::BuildMarketDepthUrl(const string symbol, int limit)
{
    return ApiServer_ + "/v2/public/orderBook/L2?symbol=" + symbol;
}

string BybitFuturesExchange::BuildAggregateTradesUrl(const string symbol, timestamp_t start_time, timestamp_t end_time, int limit)
{
    return ApiServer_ + "/v2/public/trading-records?symbol=" + symbol + "&limit=" + std::to_string(limit);
}

string BybitFuturesExchange::BuildCandlesUrl(const string symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit)
{
    return ApiServer_ + "/v2/public/index-price-kline?symbol=" + symbol + "&interval=" +
            GetInterval(tf) + "&from=" + std::to_string(start_time) + "&limit=" + std::to_string(limit);
}

string BybitFuturesExchange::BuildAccountUrl(timestamp_t timestamp)
{
    return "";
}

timestamp_t BybitFuturesExchange::ParseServerTime(const json::value& value)
{
    try {
        if (value.at("retCode").to_number<int>() == 0)
            return value.at("time").to_number<timestamp_t>();
        else
            return -1;
    } catch (std::exception& e) {
        ErrorMessage("<BybitExchange::ParseServerTime> Error parse json !!!");
        return -1;
    }
}

bool BybitFuturesExchange::ParseSymbols(const json::value& value, std::list<Symbol> &symbols)
{
    try {
        symbols.clear();
        if (value.at("ret_code").to_number<int>() != 0)
            return false;

        auto& symbols_list = value.at("result").at("list").as_array();
        for (auto& i : symbols_list) {
            auto& s = i.as_object();
            Symbol symbol(true);
            symbol.SetExchange("bybit-futures");
            symbol.Base.Name = s.at("baseCoin").as_string().c_str();
            //symbol.Base.AssetPrecision = std::atof(s.at("basePrecision").as_string().c_str());
            symbol.Quote.Name = s.at("quoteCoin").as_string().c_str();
            //symbol.QuotePrecison =  std::atof(s.at("quotePrecision").as_string().c_str());
            symbol.SetPriceStep(std::atof(s.at("priceFilter").at("tickSize").as_string().c_str()));
            symbol.SetQtyStep(std::atof(s.at("lotSizeFilter").at("qtyStep").as_string().c_str()));
            symbols.push_back(symbol);
        }
    } catch (std::exception& e) {
        ErrorMessage("<BybitExchange::ParseSymbols> Error Parse Json");
        return false;
    }

    return true;
}

bool BybitFuturesExchange::ParseTicker24(const json::value& value, std::list<Ticker24h>& tickers)
{
    try {
        tickers.clear();
        if (value.at("ret_code").to_number<int>() != 0)
            return false;

        auto& ticker_array = value.at("result").as_array();
        for (auto& i : ticker_array) {
            Ticker24h ticker;
            auto& p = i.as_object();
            ticker.Exchange = "bybit-futures";
            ticker.Symbol = p.at("symbol").as_string();
            ticker.High = std::atof(p.at("high_price_24h").as_string().c_str());
            ticker.Low = std::atof(p.at("low_price_24h").as_string().c_str());
            ticker.Open = std::atof(p.at("prev_price_24h").as_string().c_str());
            ticker.LastPrice = std::atof(p.at("last_price").as_string().c_str());
            ticker.QuoteVolume = std::atof(p.at("turnover_24h").as_string().c_str());
            ticker.Volume = std::atof(p.at("volume_24h").as_string().c_str());

            tickers.push_back(ticker);
        }
        return true;
    } catch (std::exception& e) {
        ErrorMessage("<BybitExchange::BybitFuturesExchange> Error Parse Json");
        return false;
    }

    return true;
}
