#include "binancefuturesexchange.h"

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

BinanceFuturesExchange::BinanceFuturesExchange()
{
    HOST = "fapi.binance.com";
    PORT = "443";
    API = "/fapi";
}

BinanceFuturesExchange::~BinanceFuturesExchange()
{

}

const std::string BinanceFuturesExchange::GetListenKeyEndpoint()
{
    return API + "/v1/listenKey";
}

const string BinanceFuturesExchange::GetTicker24Endpoint()
{
    return API + "/v1/ticker/24hr";
}

const string BinanceFuturesExchange::GetAccountInfoEndpoint(timestamp_t time)
{
    string query = "timestamp=" + std::to_string(time) +
            "&recvWindow=" + std::to_string(RecvWindow_);
    string signature =  hmac_sha256(SecretKey_.c_str(), query.c_str());
    return API + "/v2/account?" + query + "&signature=" + signature;
}

bool BinanceFuturesExchange::ParseSymbols(const boost::json::value& json, std::list<Symbol> &symbols)
{
    try {
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
    } catch (std::exception& e) {
        ErrorMessage(e.what());
        return false;
    }

    return true;
}

bool BinanceFuturesExchange::ParseAccountInfo(const boost::json::value& json, AccountInfo& info)
{
    try {
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
    } catch (std::exception& e) {
        ErrorMessage(e.what());
        return false;
    }

    return true;
}

bool BinanceFuturesExchange::GetMarginOptions(std::string& symbol, FuturesMarginOption &options)
{
    timestamp_t timestamp = GetServerTime();
    string url = API + "/v2/positionRisk?";

    string query("timestamp=");
    query.append(std::to_string(timestamp));

    query.append("&recvWindow=");
    query.append(std::to_string(RecvWindow_));

    query.append("&symbol=");
    query.append(symbol);

    string signature =  hmac_sha256(SecretKey_.c_str(), query.c_str());
    query.append("&signature=");
    query.append(signature);

    url += query;

    string str_result;
    GetUrl(url, str_result);
    bool ret = false;

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseMarginOptions(value, options);
        } catch (std::exception& e) {
            ErrorMessage((F("<GetMarginOptions> Error ! %s") % e.what()).str());
        }
    } else
        ErrorMessage("<GetMarginOptions> Failed to get anything.");

    return ret;
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
        ErrorMessage(e.what());
        return false;
    }

    return true;
}

bool BinanceFuturesExchange::SetMarginType(timestamp_t time, std::string& symbol, MarginType type)
{
    string url = API + "/v1/marginType?";

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

    string str_result;
    PostUrl(url, str_result);
    bool ret = false;

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = CheckSetMarginType(value);
        } catch (std::exception& e) {
            ErrorMessage((F("<GetMarginOptions> Error ! %s") % e.what()).str());
        }
    } else
        ErrorMessage("<GetMarginOptions> Failed to get anything.");

    return ret;
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

bool BinanceFuturesExchange::SetLeverage(timestamp_t time, std::string& symbol, int leverage)
{
    bool ret = false;
    string url = API + "/v1/leverage?";

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


    string action = "POST";
    string str_result;
    PostUrl(url, str_result);
    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = CheckSetLeverage(value);
        } catch (std::exception& e) {
            ErrorMessage((F("<SetLeverage> Error ! %s") % e.what()).str());
        }
    } else
        ErrorMessage("<SetLeverage> Failed to get anything.");

    return ret;
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

bool BinanceFuturesExchange::GetCurrentPosition(const std::string &symbol, std::list<Position>& pos)
{
    bool ret = false;
    timestamp_t timestamp = GetServerTime();
    string url = GetPositionEndpoint(timestamp, symbol);
    string str_result;
    GetUrl(url, str_result);
    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseCurrentPosition(value, pos);
        } catch (std::exception& e) {
            ErrorMessage((F("<GetCurrentPosition> Error ! %s") % e.what()).str());
        }
    } else
        ErrorMessage("<GetCurrentPosition> Failed to get anything.");

    return ret;
}

const string BinanceFuturesExchange::GetPositionEndpoint(timestamp_t time, const string& symbol)
{
    string url = API + "/v2/positionRisk?";

    string querystring("timestamp=");
    querystring.append(std::to_string(time));
    querystring.append("&recvWindow=");
    querystring.append(std::to_string(RecvWindow_));
    querystring.append("&symbol=");
    querystring.append(symbol);

    string signature =  hmac_sha256(SecretKey_.c_str(), querystring.c_str());
    querystring.append("&signature=");
    querystring.append(signature);

    url.append(querystring);
    return url;
}

bool BinanceFuturesExchange::ParseCurrentPosition(boost::json::value& value, std::list<Position>& pos)
{
    try {
        auto& array = value.as_array();
        pos.clear();
        for (auto& p : array) {
            Position position;
            position.EntryPrice = atof(p.at("entryPrice").as_string().c_str());
            position.Leverage = atof(p.at("leverage").as_string().c_str());
            position.LiquidationPrice = atof(p.at("liquidationPrice").as_string().c_str());
            position.MarkPrice = atof(p.at("markPrice").as_string().c_str());
            position.Symbol = p.at("symbol").as_string().c_str();
            if (p.at("marginType").as_string() == "isolated")
                position.Type = MarginType::Isolated;
            else
                position.Type = MarginType::Crossed;

            position.UnrealizedProfit = atof(p.at("unRealizedProfit").as_string().c_str());
            position.UpdateTime = p.at("updateTime").to_number<timestamp_t>();
            position.Side = String2OrderSide(p.at("positionSide").as_string().c_str());
            position.Qty = atof(p.at("positionAmt").as_string().c_str());
            pos.push_back(position);
        }
    } catch (std::exception& e) {
        ErrorMessage(e.what());
        return false;
    }

    return true;
}
