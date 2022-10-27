
#include "exchangeobj.h"
#include <vector>
#include <iostream>

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

    Log("<ExchangeObj::GetServerTime> Url: |%s|", url.c_str());
    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            server_time = ParseServerTime(value);
        } catch(std::exception& e) {
            Log("<ExchangeObj::GetServerTime> Error ! %s", e.what());
        }
    } else
        Log("<ExchangeObj::GetServerTime> Failed to get anything.");

    curl_easy_reset(curl);
    return server_time;
}

bool ExchangeObj::GetExchangeInfo(ExchangeInfo &info)
{
    string str_result;
    string url = BuildExchangeInfoUrl();
    GetUrl(url, str_result);
    bool ret = false;;

    Log("<ExchangeObj::GetExchangeInfo> Url: |%s|", url.c_str());
    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseExchangeInfo(value, info);
        } catch (std::exception& e) {
            Log("<ExchangeObj::GetExchangeInfo> Error ! %s", e.what());
        }
    } else
        Log("<ExchangeObj::GetExchangeInfo> Failed to get anything.");


    curl_easy_reset(curl);
    return ret;
}

bool ExchangeObj::GetAllPrices(Prices &prices)
{
    Log("<ExchangeObj::GetAllPrices>");
    bool ret = false;

    string str_result;
    string url = BuildAllPricesUrl();
    GetUrl(url, str_result);

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseAllPrices(value, prices);
        } catch (std::exception& e) {
            Log("<ExchangeObj::GetAllPrices> Error ! %s", e.what());
        }
    } else {
        Log("<ExchangeObj::GetAllPrices> Failed to get anything.");
    }

    curl_easy_reset(curl);

    return ret;
}

bool ExchangeObj::GetMarketDepth(const string& symbol, int limit, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId)
{
    Log("<ExchangeObj::GetMarketDepth>");
    bool ret = false;

    string str_result;
    string url = BuildMarketDepthUrl(symbol, limit);
    GetUrl(url, str_result);

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseMarketDepth(value, Asks, Bids, lastUpdateId);
        } catch (std::exception& e) {
            Log("<ExchangeObj::GetMarketDepth> Error ! %s", e.what());
        }
    } else {
        Log("<ExchangeObj::GetMarketDepth> Failed to get anything.");
    }

    curl_easy_reset(curl);

    return ret;
}

bool ExchangeObj::GetTrades(const std::string& symbol, timestamp_t start_time, timestamp_t end_time, int limit, TradesList& trades)
{
    Log("<ExchangeObj::GetTrades>");
    bool ret = false;

    string str_result;
    string url = BuildAggregateTradesUrl(symbol, start_time, end_time, limit);
    GetUrl(url, str_result);

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseAggregateTradesList(value, trades);
        } catch (std::exception& e) {
            Log("<ExchangeObj::GetTrades> Error ! %s", e.what());
        }
    } else {
        Log("<ExchangeObj::GetTrades> Failed to get anything.");
    }

    curl_easy_reset(curl);

    return ret;
}

bool ExchangeObj::GetCandles(const string& symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit, CandlesList &candles)
{
    Log("<ExchangeObj::GetCandles>");
    bool ret = false;

    string str_result;
    string url = BuildCandlesUrl(symbol, tf, start_time, end_time, limit);
    GetUrl(url, str_result);

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseCandles(value, candles);
        } catch (std::exception& e) {
            Log("<ExchangeObj::GetCandles> Error ! %s", e.what());
        }
    } else {
        Log("<ExchangeObj::GetCandles> Failed to get anything.");
    }

    curl_easy_reset(curl);

    return ret;
}

bool ExchangeObj::GetAccount(AccountInfo &info)
{
    bool ret = false;
    Log("<ExchangeObj::GetAccount>");
    if (ApiKey_.size() == 0 || SecretKey_.size() == 0) {
        Log("<ExchangeObj::GetAccount> API Key and Secret Key has not been set.");
        return false;
    }

    timestamp_t timestamp = GetServerTime();
    string url = BuildAccountUrl(timestamp);
    vector<string> extra_http_header;
    string header_chunk("X-MBX-APIKEY: ");
    header_chunk.append(ApiKey_);
    extra_http_header.push_back(header_chunk);

    Log("<ExchangeObj::GetAccount> url = |%s|" , url.c_str());
    string post_data = "";

    string str_result;
    string action = "GET";
    GetUrlWithHeader(url, str_result, extra_http_header, post_data, action);
    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseAccount(value, info);
        } catch (std::exception &e) {
            Log("<ExchangeObj::GetAccount> Error ! %s", e.what());
        }
    } else {
        Log("<ExchangeObj::GetAccount> Failed to get anything.");
    }

    curl_easy_reset(curl);
    return ret;
}

void ExchangeObj::Cleanup()
{
    curl_easy_cleanup(curl);
    curl_global_cleanup();
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
            Log("<ExchangeObj::curl_api> curl_easy_perform() failed: %s" , curl_easy_strerror(res)) ;
        }
    }
}

void ExchangeObj::Log(const char* msg, ...)
{
    if (Logger_ == nullptr)
        return;

    Logger_->Log(msg);
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
        Log("<Error> Json Object is null !!!!");
        return true;
    }

    try {
        int code = result.at("code").to_number<int>();
        Log("<Error> Code ", code);
        return true;
    } catch (...) {
        return false;
    }

    return false;
}
