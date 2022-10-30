#include "binancefuturesexchange.h"

BinanceFuturesExchange::BinanceFuturesExchange()
{
    ApiType_ = API_PATH;
    ApiServer_ = BINANCE_FUTURES_SERVER;
}

BinanceFuturesExchange::~BinanceFuturesExchange()
{

}

bool BinanceFuturesExchange::ParseAccount(const json::value &json, AccountInfo &info)
{
    info.Balance.clear();
    info.AccountType = "FUTURES";
    for (auto b : json.at("assets").as_array()) {
        Balance balance;
        balance.Asset = b.at("asset").as_string().c_str();
        balance.Free = std::stod(b.at("availableBalance").as_string().c_str());
        balance.Locked = std::stod(b.at("walletBalance").as_string().c_str()) -
                std::stod(b.at("unrealizedProfit").as_string().c_str());
        info.Balance.push_back(balance);
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

bool BinanceFuturesExchange::ParseSymbols(const json::value &json, std::list<Symbol> &symbols)
{
    symbols.clear();
    for (auto i : json.at("symbols").as_array()) {
        if (i.at("status") == "TRADING") {
            Symbol s(true);
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
