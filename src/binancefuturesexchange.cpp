#include "binancefuturesexchange.h"
#include <boost/format.hpp>

BinanceFuturesExchange::BinanceFuturesExchange()
{
    ApiType_ = API_PATH;
    ApiServer_ = BINANCE_FUTURES_SERVER;
}

BinanceFuturesExchange::~BinanceFuturesExchange()
{

}

bool BinanceFuturesExchange::SetMarginOptions(std::string& symbol, FuturesMarginOption &options)
{
    timestamp_t timestamp = GetServerTime();
    if (timestamp < 0)
        return false;

    if (!SetMarginType(timestamp, symbol, options.Type))
        return false;

    if (!SetLeverage(timestamp, symbol, options.Leverage))
        return false;

    return true;
}

bool BinanceFuturesExchange::SetMarginType(timestamp_t time, std::string& symbol, MarginType type)
{
    bool ret = false;
    string url = ApiServer_;
    url += ApiType_ + "/v1/marginType?";

    string querystring("timestamp=");
    querystring.append(std::to_string(time));

    if (RecvWindow_ > 0) {
        querystring.append("&recvWindow=");
        querystring.append(std::to_string(RecvWindow_));
    }

    querystring.append("&symbol=");
    querystring.append(symbol);

    querystring.append("&marginType=");
    if (type == MarginType::Crossed)
        querystring.append("CROSSED");
    else
        querystring.append("ISOLATED");

    string signature =  hmac_sha256(SecretKey_.c_str(), querystring.c_str());
    querystring.append("&signature=");
    querystring.append(signature);

    url.append(querystring);

    InfoMessage("<BinanceFuturesExchange::SetMarginType>");
    if (ApiKey_.size() == 0 || SecretKey_.size() == 0) {
        WarningMessage("<BinanceFuturesExchange::GetMarginType> API Key and Secret Key has not been set.");
        return false;
    }

    vector<string> extra_http_header;
    string header_chunk("X-MBX-APIKEY: ");
    header_chunk.append(ApiKey_);
    extra_http_header.push_back(header_chunk);

    InfoMessage((boost::format("<BinanceFuturesExchange::SetMarginType> url = |%s|") % url.c_str()).str());
    string post_data = "";

    string str_result;
    string action = "POST";
    GetUrlWithHeader(url, str_result, extra_http_header, post_data, action);
    if (str_result.size() > 0) {
        try {
            boost::json::stream_parser parser;
            parser.write(str_result);
            auto value = parser.release();
            if (IsError(value)) {
                curl_easy_reset(curl);
                return false;
            }
            ret = CheckSetMarginType(value);
        } catch (std::exception &e) {
            ErrorMessage((boost::format("<BinanceFuturesExchange::SetMarginType> Error ! %s") % e.what()).str());
        }
    } else
        ErrorMessage("<BinanceFuturesExchange::SetMarginType> Failed to get anything.");

    curl_easy_reset(curl);
    return ret;
}

bool BinanceFuturesExchange::SetLeverage(timestamp_t time, std::string& symbol, int leverage)
{
    bool ret = false;
    string url = ApiServer_;
    url += ApiType_ + "/v1/leverage?";

    string querystring("timestamp=");
    querystring.append(std::to_string(time));

    if (RecvWindow_ > 0) {
        querystring.append("&recvWindow=");
        querystring.append(std::to_string(RecvWindow_));
    }

    querystring.append("&symbol=");
    querystring.append(symbol);

    querystring.append("&leverage=");
    querystring.append(std::to_string(leverage));

    string signature =  hmac_sha256(SecretKey_.c_str(), querystring.c_str());
    querystring.append("&signature=");
    querystring.append(signature);

    url.append(querystring);

    InfoMessage("<BinanceFuturesExchange::SetLeverage>");
    if (ApiKey_.size() == 0 || SecretKey_.size() == 0) {
        WarningMessage("<BinanceFuturesExchange::SetLeverage> API Key and Secret Key has not been set.");
        return false;
    }

    vector<string> extra_http_header;
    string header_chunk("X-MBX-APIKEY: ");
    header_chunk.append(ApiKey_);
    extra_http_header.push_back(header_chunk);

    InfoMessage((boost::format("<BinanceFuturesExchange::SetLeverage> url = |%s|") % url.c_str()).str());
    string post_data = "";

    string str_result;
    string action = "POST";
    GetUrlWithHeader(url, str_result, extra_http_header, post_data, action);
    if (str_result.size() > 0) {
        try {
            boost::json::stream_parser parser;
            parser.write(str_result);
            auto value = parser.release();
            if (IsError(value)) {
                curl_easy_reset(curl);
                return false;
            }
            ret = CheckSetLeverage(value);
        } catch (std::exception &e) {
            ErrorMessage((boost::format("<BinanceFuturesExchange::SetLeverage> Error ! %s") % e.what()).str());
        }
    } else
        ErrorMessage("<BinanceFuturesExchange::SetLeverage> Failed to get anything.");

    curl_easy_reset(curl);
    return ret;
}

bool BinanceFuturesExchange::GetMarginOptions(std::string& symbol, FuturesMarginOption &options)
{
    bool ret = false;
    timestamp_t timestamp = GetServerTime();
    string url = ApiServer_;
    url += ApiType_ + "/v2/positionRisk?";

    string querystring("timestamp=");
    querystring.append(std::to_string(timestamp));

    if (RecvWindow_ > 0) {
        querystring.append("&recvWindow=");
        querystring.append(std::to_string(RecvWindow_));
    }

    querystring.append("&symbol=");
    querystring.append(symbol);

    string signature =  hmac_sha256(SecretKey_.c_str(), querystring.c_str());
    querystring.append("&signature=");
    querystring.append(signature);

    url.append(querystring);

    InfoMessage("<BinanceFuturesExchange::GetMarginType>");
    if (ApiKey_.size() == 0 || SecretKey_.size() == 0) {
        WarningMessage("<BinanceFuturesExchange::GetMarginType> API Key and Secret Key has not been set.");
        return false;
    }

    vector<string> extra_http_header;
    string header_chunk("X-MBX-APIKEY: ");
    header_chunk.append(ApiKey_);
    extra_http_header.push_back(header_chunk);

    InfoMessage((boost::format("<BinanceFuturesExchange::GetMarginType> url = |%s|") % url.c_str()).str());
    string post_data = "";

    string str_result;
    string action = "GET";
    GetUrlWithHeader(url, str_result, extra_http_header, post_data, action);
    if (str_result.size() > 0) {
        try {
            boost::json::stream_parser parser;
            parser.write(str_result);
            auto value = parser.release();
            if (IsError(value)) {
                curl_easy_reset(curl);
                return false;
            }
            ret = ParseMarginOptions(value, options);
        } catch (std::exception &e) {
            ErrorMessage((boost::format("<BinanceFuturesExchange::GetMarginType> Error ! %s") % e.what()).str());
        }
    } else
        ErrorMessage("<BinanceFuturesExchange::GetMarginType> Failed to get anything.");

    curl_easy_reset(curl);
    return ret;
}

bool BinanceFuturesExchange::ParseAccount(const json::value &json, AccountInfo &info)
{
    info.Balance.clear();
    info.AccountType = "FUTURES";
    for (auto& b : json.at("assets").as_array()) {
        auto& b1 = b.as_object();
        Balance balance;
        balance.Asset = b1.at("asset").as_string().c_str();
        balance.Free = std::stod(b1.at("availableBalance").as_string().c_str());
        balance.Locked = std::stod(b1.at("walletBalance").as_string().c_str()) -
                std::stod(b1.at("availableBalance").as_string().c_str());
        info.Balance.push_back(balance);
    }

    return true;
}

bool BinanceFuturesExchange::CheckSetMarginType(const json::value &json)
{
    try {
        int code = json.at("code").to_number<int>();
        if (code == 200 || code == -4046)
            return true;
        else
            return false;
    } catch (std::exception& e) {
        return false;
    }

    return true;
}

bool BinanceFuturesExchange::CheckSetLeverage(const json::value &json)
{
    try {
        int leverage = json.at("leverage").to_number<int>();
        if (leverage > 0)
            return true;
        else
            return false;
    } catch (std::exception& e) {
        return false;
    }

    return true;
}

string BinanceFuturesExchange::BuildAccountUrl(timestamp_t timestamp)
{
    string url = ApiServer_;
    url += ApiType_ + "/v1/account?";

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

string BinanceFuturesExchange::BuildTicker24Url()
{
    return ApiServer_ + ApiType_ + "/v1/ticker/24hr";
}

bool BinanceFuturesExchange::ParseSymbols(const json::value &json, std::list<Symbol> &symbols)
{
    symbols.clear();
    for (auto& i : json.at("symbols").as_array()) {
        if (i.at("status") == "TRADING") {
            Symbol s(true);
            s.SetExchange("binance-futures");
            s.Base.AssetPrecision = i.at("baseAssetPrecision").to_number<int>();
            //s.Base.ComissionPrecision = i.at("baseCommissionPrecision").to_number<int>();
            s.Base.Name = i.at("baseAsset").as_string().c_str();

            //s.Quote.AssetPrecision = i.at("quoteAssetPrecision").to_number<int>();
            //s.Quote.ComissionPrecision = i.at("quoteCommissionPrecision").to_number<int>();
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

bool BinanceFuturesExchange::ParseMarginOptions(const json::value& value, FuturesMarginOption &options)
{
    try {
        auto& array = value.as_array();
        for (auto& p : array) {
            options.Leverage = std::atoi(p.at("leverage").as_string().c_str());
            if (p.at("marginType").as_string() == "isolated")
                options.Type = MarginType::Isolated;
            else
                options.Type = MarginType::Crossed;
        }
    } catch (std::exception& e) {
        return false;
    }

    return true;
}

bool BinanceFuturesExchange::ParseTicker24(const json::value& value, std::list<Ticker24h>& tickers)
{
    tickers.clear();
    for (auto& p : value.as_array()) {
        Ticker24h ticker;
        ticker.Exchange = "binance-futures";

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
