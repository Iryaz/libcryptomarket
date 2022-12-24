
#include "exchangeobj.h"
#include <boost/format.hpp>
#include <vector>
#include <iostream>
#include <hmac.h>
#include <sha.h>

#define F(S) boost::format(S)

#define JSON_PARSE \
    boost::json::stream_parser parser; \
    parser.write(str_result); \
    auto value = parser.release(); \
    if (IsError(value)) { \
        curl_easy_reset(curl); \
        return -1; \
    } \

ExchangeObj::ExchangeObj()
{
    RecvWindow_ = 2000;
    Logger_ = nullptr;

    CUrlCallback_= [](char *content, size_t size, size_t nmemb, std::string *buffer) {
        buffer->append((char*)content, size*nmemb);
        return size*nmemb;
    };
}

ExchangeObj::~ExchangeObj()
{
    curl_global_cleanup();
}

void ExchangeObj::Init(string api, string secret)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    ApiKey_ = api;
    SecretKey_ = secret;
}

timestamp_t ExchangeObj::GetServerTime()
{
    string str_result;
    string url = BuildTimeUrl();
    GetUrl(url, str_result);
    timestamp_t server_time = -1;

    InfoMessage((F("<ExchangeObj::GetServerTime> Url: |%s|") % url.c_str()).str());
    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            server_time = ParseServerTime(value);
        } catch(std::exception& e) {
            ErrorMessage((F("<ExchangeObj::GetServerTime> Error ! %s") % e.what()).str());
        }
    } else
        ErrorMessage("<ExchangeObj::GetServerTime> Failed to get anything.");

    curl_easy_reset(curl);
    return server_time;
}

bool ExchangeObj::GetSymbols(std::list<Symbol> &symbols)
{
    string str_result;
    string url = BuildSymbolsUrl();
    GetUrl(url, str_result);
    bool ret = false;;

    InfoMessage((F("<ExchangeObj::GetSymbols> Url: |%s|") % url.c_str()).str());
    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseSymbols(value, symbols);
        } catch (std::exception& e) {
            ErrorMessage((F("<ExchangeObj::GetSymbols> Error ! %s") % e.what()).str());
        }
    } else
        ErrorMessage("<ExchangeObj::GetSymbols> Failed to get anything.");


    curl_easy_reset(curl);
    return ret;
}

bool ExchangeObj::GetTicker24(std::list<Ticker24h>& tickers)
{
    InfoMessage("<ExchangeObj::GetTicker24>");
    bool ret = false;

    string str_result;
    string url = BuildTicker24Url();
    GetUrl(url, str_result);

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseTicker24(value, tickers);
        } catch (std::exception& e) {
            ErrorMessage((F("<ExchangeObj::GetTicker24> Error ! %s") % e.what()).str());
        }
    } else {
        ErrorMessage("<ExchangeObj::GetTicker24> Failed to get anything.");
    }

    curl_easy_reset(curl);

    return ret;
}

bool ExchangeObj::GetMarketDepth(const string& symbol, int limit, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId)
{
    InfoMessage("<ExchangeObj::GetMarketDepth>");
    bool ret = false;

    string str_result;
    string url = BuildMarketDepthUrl(symbol, limit);
    GetUrl(url, str_result);

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseMarketDepth(value, Asks, Bids, lastUpdateId);
        } catch (std::exception& e) {
            ErrorMessage((F("<ExchangeObj::GetMarketDepth> Error ! %s") % e.what()).str());
        }
    } else {
        ErrorMessage("<ExchangeObj::GetMarketDepth> Failed to get anything.");
    }

    curl_easy_reset(curl);

    return ret;
}

bool ExchangeObj::GetTrades(const std::string& symbol, timestamp_t start_time, timestamp_t end_time, int limit, TradesList& trades)
{
    InfoMessage("<ExchangeObj::GetTrades>");
    bool ret = false;

    string str_result;
    string url = BuildAggregateTradesUrl(symbol, start_time, end_time, limit);
    GetUrl(url, str_result);

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseAggregateTradesList(value, trades);
        } catch (std::exception& e) {
            ErrorMessage((F("<ExchangeObj::GetTrades> Error ! %s") % e.what()).str());
        }
    } else {
        ErrorMessage("<ExchangeObj::GetTrades> Failed to get anything.");
    }

    curl_easy_reset(curl);

    return ret;
}

bool ExchangeObj::GetCandles(const string& symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit, CandlesList &candles)
{
    InfoMessage("<ExchangeObj::GetCandles>");
    bool ret = false;

    string str_result;
    string url = BuildCandlesUrl(symbol, tf, start_time, end_time, limit);
    GetUrl(url, str_result);

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseCandles(value, candles);
        } catch (std::exception& e) {
            ErrorMessage((F("<ExchangeObj::GetCandles> Error ! %s") % e.what()).str());
        }
    } else {
        ErrorMessage("<ExchangeObj::GetCandles> Failed to get anything.");
    }

    curl_easy_reset(curl);

    return ret;
}

bool ExchangeObj::GetAccount(AccountInfo &info)
{
    bool ret = false;
    InfoMessage("<ExchangeObj::GetAccount>");
    if (ApiKey_.size() == 0 || SecretKey_.size() == 0) {
        WarningMessage("<ExchangeObj::GetAccount> API Key and Secret Key has not been set.");
        return false;
    }

    timestamp_t timestamp = GetServerTime();
    string url = BuildAccountUrl(timestamp);
    vector<string> extra_http_header;
    string header_chunk("X-MBX-APIKEY: ");
    header_chunk.append(ApiKey_);
    extra_http_header.push_back(header_chunk);

    InfoMessage((F("<ExchangeObj::GetAccount> url = |%s|") % url.c_str()).str());
    string post_data = "";

    string str_result;
    string action = "GET";
    GetUrlWithHeader(url, str_result, extra_http_header, post_data, action);
    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseAccount(value, info);
        } catch (std::exception &e) {
            ErrorMessage((F("<ExchangeObj::GetAccount> Error ! %s") % e.what()).str());
        }
    } else {
        ErrorMessage("<ExchangeObj::GetAccount> Failed to get anything.");
    }

    curl_easy_reset(curl);
    return ret;
}

bool ExchangeObj::GetListenKey(std::string& key)
{
    bool ret = false;
    InfoMessage("<ExchangeObj::GetListenKey>");
    if (ApiKey_.size() == 0) {
        WarningMessage("<ExchangeObj::GetListenKey> API Key and Secret Key has not been set.");
        return false;
    }

    string url = GetListenKeyUrl();
    vector<string> extra_http_header;
    string header_chunk("X-MBX-APIKEY: ");
    header_chunk.append(ApiKey_);
    extra_http_header.push_back(header_chunk);

    InfoMessage((F("<ExchangeObj::GetListenKey> url = |%s|") % url.c_str()).str());
    string post_data = "";

    string str_result;
    string action = "POST";
    GetUrlWithHeader(url, str_result, extra_http_header, post_data, action);
    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseListenKey(value, key);
        } catch (std::exception &e) {
            ErrorMessage((F("<ExchangeObj::GetListenKey> Error ! %s") % e.what()).str());
        }
    } else {
        ErrorMessage("<ExchangeObj::GetListenKey> Failed to get anything.");
    }

    curl_easy_reset(curl);
    return ret;
}

bool ExchangeObj::PutListenKey(const std::string& key)
{
    bool ret = false;
    InfoMessage("<ExchangeObj::PutListenKey>");
    if (ApiKey_.size() == 0) {
        WarningMessage("<ExchangeObj::PutListenKey> API Key and Secret Key has not been set.");
        return false;
    }

    string url = PutListenKeyUrl(key);
    vector<string> extra_http_header;
    string header_chunk("X-MBX-APIKEY: ");
    header_chunk.append(ApiKey_);
    extra_http_header.push_back(header_chunk);

    InfoMessage((F("<ExchangeObj::PutListenKey> url = |%s|") % url.c_str()).str());
    string post_data = "";

    string str_result;
    string action = "PUT";
    GetUrlWithHeader(url, str_result, extra_http_header, post_data, action);
    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = true;
        } catch (std::exception &e) {
            ErrorMessage((F("<ExchangeObj::PutListenKey> Error ! %s") % e.what()).str());
        }
    } else {
        ErrorMessage("<ExchangeObj::PutListenKey> Failed to get anything.");
    }

    curl_easy_reset(curl);
    return ret;
}

bool ExchangeObj::CloseListenKey(const std::string& key)
{
    bool ret = false;
    InfoMessage("<ExchangeObj::CloseListenKey>");
    if (ApiKey_.size() == 0) {
        WarningMessage("<ExchangeObj::CloseListenKey> API Key and Secret Key has not been set.");
        return false;
    }

    string url = PutListenKeyUrl(key);
    vector<string> extra_http_header;
    string header_chunk("X-MBX-APIKEY: ");
    header_chunk.append(ApiKey_);
    extra_http_header.push_back(header_chunk);

    InfoMessage((F("<ExchangeObj::CloseListenKey> url = |%s|") % url.c_str()).str());
    string post_data = "";

    string str_result;
    string action = "DELETE";
    GetUrlWithHeader(url, str_result, extra_http_header, post_data, action);
    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = true;
        } catch (std::exception &e) {
            ErrorMessage((F("<ExchangeObj::CloseListenKey> Error ! %s") % e.what()).str());
        }
    } else {
        ErrorMessage("<ExchangeObj::CloseListenKey> Failed to get anything.");
    }

    curl_easy_reset(curl);
    return ret;
}

bool ExchangeObj::GetOpenOrders(OrderList& orders)
{
    bool ret = false;
    InfoMessage("<ExchangeObj::GetAllOrders>");
    if (ApiKey_.size() == 0 || SecretKey_.size() == 0) {
        WarningMessage("<ExchangeObj::GetAllOrders> API Key and Secret Key has not been set.");
        return false;
    }

    timestamp_t timestamp = GetServerTime();
    string url = BuildOpenOrdersUrl(timestamp);
    vector<string> extra_http_header;
    string header_chunk("X-MBX-APIKEY: ");
    header_chunk.append(ApiKey_);
    extra_http_header.push_back(header_chunk);

    InfoMessage((F("<ExchangeObj::GetAllOrders> url = |%s|") % url.c_str()).str());
    string post_data = "";

    string str_result;
    string action = "GET";
    GetUrlWithHeader(url, str_result, extra_http_header, post_data, action);
    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseOpenOrders(value, orders);
        } catch (std::exception &e) {
            ErrorMessage((F("<ExchangeObj::GetAllOrders> Error ! %s") % e.what()).str());
        }
    } else {
        ErrorMessage("<ExchangeObj::GetAllOrders> Failed to get anything.");
    }

    curl_easy_reset(curl);
    return ret;
}

bool ExchangeObj::NewOrder(OrderType type, std::string& symbol, Direct direct, double qty, double price, double stopPrice, Order& newOrder)
{
    bool ret = false;
    InfoMessage("<ExchangeObj::NewOrder>");
    if (ApiKey_.size() == 0 || SecretKey_.size() == 0) {
        WarningMessage("<ExchangeObj::NewOrder> API Key and Secret Key has not been set.");
        return false;
    }

    timestamp_t timestamp = GetServerTime();
    string url = BuildNewOrderUrl(timestamp, symbol, type, direct, qty, price, stopPrice);
    vector<string> extra_http_header;
    string header_chunk("X-MBX-APIKEY: ");
    header_chunk.append(ApiKey_);
    extra_http_header.push_back(header_chunk);

    InfoMessage((F("<ExchangeObj::NewOrder> url = |%s|") % url.c_str()).str());
    string post_data = "";

    string str_result;
    string action = "POST";
    GetUrlWithHeader(url, str_result, extra_http_header, post_data, action);
    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseNewOrder(value, newOrder);
        } catch (std::exception &e) {
            ErrorMessage((F("<ExchangeObj::NewOrder> Error ! %s") % e.what()).str());
        }
    } else {
        ErrorMessage("<ExchangeObj::NewOrder> Failed to get anything.");
    }

    curl_easy_reset(curl);
    return ret;
}

bool ExchangeObj::CancelOrder(Order &order)
{
    bool ret = false;
    InfoMessage("<ExchangeObj::CancelOrder>");
    if (ApiKey_.size() == 0 || SecretKey_.size() == 0) {
        WarningMessage("<ExchangeObj::CancelOrder> API Key and Secret Key has not been set.");
        return false;
    }

    timestamp_t timestamp = GetServerTime();
    string url = BuildCancelOrderUrl(timestamp, order);
    vector<string> extra_http_header;
    string header_chunk("X-MBX-APIKEY: ");
    header_chunk.append(ApiKey_);
    extra_http_header.push_back(header_chunk);

    InfoMessage((F("<ExchangeObj::CancelOrder> url = |%s|") % url.c_str()).str());
    string post_data = "";

    string str_result;
    string action = "DELETE";
    GetUrlWithHeader(url, str_result, extra_http_header, post_data, action);
    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseCancelOrder(value);
        } catch (std::exception &e) {
            ErrorMessage((F("<ExchangeObj::CancelOrder> Error ! %s") % e.what()).str());
        }
    } else {
        ErrorMessage("<ExchangeObj::CancelOrder> Failed to get anything.");
    }

    curl_easy_reset(curl);
    return ret;
}

void ExchangeObj::Cleanup()
{
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

string ExchangeObj::hmac_sha256(const char *key, const char *data)
{
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), key, strlen(key), (unsigned char*)data, strlen(data), nullptr, nullptr);
    return b2a_hex((char *)digest, 32);
}

string ExchangeObj::b2a_hex(char *byte_arr, int n)
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

void ExchangeObj::GetUrl(string &url, string &result_json)
{
    vector<string> v;
    string action = "GET";
    string post_data = "";
    GetUrlWithHeader(url, result_json, v, post_data, action);
}

void ExchangeObj::GetUrlWithHeader(string &url, string &str_result, vector<string> &extra_http_header, string &post_data, string &action)
{
    CURLcode res;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CUrlCallback_);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str_result);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip");

        if (extra_http_header.size() > 0) {
            struct curl_slist *chunk = nullptr;
            for (int i = 0; i < extra_http_header.size(); i++)
                chunk = curl_slist_append(chunk, extra_http_header[i].c_str());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        }

        if (post_data.size() > 0 || action == "POST" || action == "PUT" || action == "DELETE") {

            if (action == "PUT" || action == "DELETE")
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, action.c_str());

            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
        }

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            ErrorMessage((F("<ExchangeObj::curl_api> curl_easy_perform() failed: %s") % curl_easy_strerror(res)).str());
        }
    }
}

void ExchangeObj::ErrorMessage(const std::string &msg)
{
    Log(BaseLogger::Level::Critical, msg);
}

void ExchangeObj::InfoMessage(const std::string &msg)
{
    Log(BaseLogger::Level::Info, msg);
}

void ExchangeObj::WarningMessage(const std::string &msg)
{
    Log(BaseLogger::Level::Warning, msg);
}

void ExchangeObj::Log(BaseLogger::Level lv, const std::string &msg)
{
    if (Logger_ == nullptr)
        return;

    Logger_->Log(lv, msg);
}

string ExchangeObj::Timeframe2String(TimeFrame tf)
{
    switch (tf) {
    case TimeFrame_1m:
        return "1m";
    case TimeFrame_5m:
        return "5m";
    case TimeFrame_15m:
        return "15m";
    case TimeFrame_1h:
        return "1h";
    case TimeFrame_4h:
        return "4h";
    case TimeFrame_1d:
        return "1d";
    }

    return "15m";
}

bool ExchangeObj::IsError(const boost::json::value& result)
{
    if (result.is_array())
        return false;

    if (result.is_null()) {
        ErrorMessage("<Error> Json Object is null !!!!");
        return true;
    }

    try {
        int code = result.at("code").to_number<int>();
        ErrorMessage((F("<Error> Code: ") % code).str());
        return true;
    } catch (...) {
        return false;
    }

    return false;
}
