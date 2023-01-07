
#include "exchangeobj.h"
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <vector>
#include <iostream>
#include <hmac.h>
#include <sha.h>

#include "cert.h"

#define F(S) boost::format(S)

#define JSON_PARSE \
    boost::json::stream_parser parser; \
    parser.write(str_result); \
    auto value = parser.release(); \
    if (IsError(value)) { \
        return -1; \
    } \

ExchangeObj::ExchangeObj()
{
    RecvWindow_ = 2000;
    Logger_ = nullptr;
}

ExchangeObj::~ExchangeObj()
{

}

void ExchangeObj::Init(string api, string secret)
{
    ApiKey_ = api;
    SecretKey_ = secret;
}

timestamp_t ExchangeObj::GetServerTime()
{
    string str_result;
    string url = GetTimeEndpoint();
    GetUrl(url, str_result);
    timestamp_t server_time = -1;

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            server_time = ParseServerTime(value);
        } catch(std::exception& e) {
            ErrorMessage((F("<GetServerTime> Error ! %s") % e.what()).str());
            return -1;
        }
    } else {
        ErrorMessage("<GetServerTime> Failed to get anything.");
    }

    return server_time;
}

bool ExchangeObj::GetSymbols(std::list<Symbol> &symbols)
{
    string str_result;
    string url = GetSymbolsEndpoint();
    GetUrl(url, str_result);
    bool ret = false;;

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseSymbols(value, symbols);
        } catch (std::exception& e) {
            ErrorMessage((F("<GetSymbols> Error ! %s") % e.what()).str());
            return false;
        }
    } else
        ErrorMessage("<GetSymbols> Failed to get anything.");

    return ret;
}

bool ExchangeObj::GetTicker24(std::list<Ticker24h>& tickers)
{
    string str_result;
    string url = GetTicker24Endpoint();
    GetUrl(url, str_result);
    bool ret = false;;

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseTicker24(value, tickers);
        } catch (std::exception& e) {
            ErrorMessage((F("<GetTicker24> Error ! %s") % e.what()).str());
            return false;
        }
    } else
        ErrorMessage("<GetTicker24> Failed to get anything.");

    return ret;
}

bool ExchangeObj::GetMarketDepth(const string &symbol, int limit, MarketDepth& Asks, MarketDepth& Bids, uint64_t& lastUpdateId)
{
    string str_result;
    string url = GetMarketDepthEndpoint(symbol, limit);
    GetUrl(url, str_result);
    bool ret = false;;

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseMarketDepth(value, Asks, Bids, lastUpdateId);
        } catch (std::exception& e) {
            ErrorMessage((F("<GetMarketDepth> Error ! %s") % e.what()).str());
            return false;
        }
    } else
        ErrorMessage("<GetMarketDepth> Failed to get anything.");

    return ret;
}

bool ExchangeObj::GetTrades(const string& symbol, timestamp_t start_time, timestamp_t end_time, int limit, TradesList& trades)
{
    string str_result;
    string url = GetTradesEndpoint(symbol, start_time, end_time, limit);
    GetUrl(url, str_result);
    bool ret = false;;

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseTrades(value, trades);
        } catch (std::exception& e) {
            ErrorMessage((F("<GetTrades> Error ! %s") % e.what()).str());
            return false;
        }
    } else
        ErrorMessage("<GetTrades> Failed to get anything.");

    return ret;
}

bool ExchangeObj::GetCandles(const string& symbol, TimeFrame tf, timestamp_t start_time, timestamp_t end_time, int limit, CandlesList& candles)
{
    string str_result;
    string url = GetCandlesEndpoint(symbol, tf, start_time, end_time, limit);
    GetUrl(url, str_result);
    bool ret = false;;

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseCandles(value, candles);
        } catch (std::exception& e) {
            ErrorMessage((F("<GetTrades> Error ! %s") % e.what()).str());
            return false;
        }
    } else
        ErrorMessage("<GetTrades> Failed to get anything.");

    return ret;
}

bool ExchangeObj::GetAccount(AccountInfo &info)
{
    timestamp_t time = GetServerTime();
    if (static_cast<long long>(time) == -1) {
        ErrorMessage("<GetAccount> Error ServerTime !!!!");
        return false;
    }

    string str_result;
    string url = GetAccountInfoEndpoint(time);
    GetUrl(url, str_result);
    bool ret = false;

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseAccountInfo(value, info);
        } catch (std::exception& e) {
            ErrorMessage((F("<GetAccount> Error ! %s") % e.what()).str());
        }
    } else
        ErrorMessage("<GetAccount> Failed to get anything.");

    return ret;
}

bool ExchangeObj::GetOpenOrders(OrderList& orders)
{
    timestamp_t time = GetServerTime();
    if (static_cast<long long>(time) == -1) {
        ErrorMessage("<GetOpenOrders> Error ServerTime !!!!");
        return false;
    }

    string str_result;
    string url = GetOpenOrdersEndpoint(time);
    GetUrl(url, str_result);
    bool ret = false;

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseOrders(value, orders);
        } catch (std::exception& e) {
            ErrorMessage((F("<GetOpenOrders> Error ! %s") % e.what()).str());
        }
    } else
        ErrorMessage("<GetOpenOrders> Failed to get anything.");

    return ret;
}

bool ExchangeObj::NewOrder(OrderType type, std::string& symbol, Direct direct, double qty, double price, double stopPrice, Order& newOrder)
{
    timestamp_t time = GetServerTime();
    if (static_cast<long long>(time) == -1) {
        ErrorMessage("<NewOrder> Error ServerTime !!!!");
        return false;
    }

    string str_result;
    string url = GetNewOrderEndpoint(time, symbol, direct, type, qty, price, stopPrice);
    PostUrl(url, str_result);
    bool ret = false;

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseNewOrder(value, newOrder);
        } catch (std::exception& e) {
            ErrorMessage((F("<NewOrder> Error ! %s") % e.what()).str());
        }
    } else
        ErrorMessage("<NewOrder> Failed to get anything.");

    return ret;
}

bool ExchangeObj::CancelOrder(Order& order)
{
    timestamp_t time = GetServerTime();
    if (static_cast<long long>(time) == -1) {
        ErrorMessage("<CancelOrder> Error ServerTime !!!!");
        return false;
    }

    string str_result;
    string url = GetCancelOrderEndpoint(time, order);
    DeleteUrl(url, str_result);
    bool ret = false;

    if (str_result.size() > 0) {
        try {
            JSON_PARSE
            ret = ParseCancelOrder(value);
        } catch (std::exception& e) {
            ErrorMessage((F("<CancelOrder> Error ! %s") % e.what()).str());
        }
    } else
        ErrorMessage("<CancelOrder> Failed to get anything.");

    return ret;
}

void ExchangeObj::GetUrl(const string& path, string &result_json)
{
    GetUrlWithHeader(path, boost::beast::http::verb::get, result_json);
}

void ExchangeObj::PutUrl(const string& path, string &result_json)
{
    GetUrlWithHeader(path, boost::beast::http::verb::put, result_json);
}

void ExchangeObj::PostUrl(const string& path, string &result_json)
{
    GetUrlWithHeader(path, boost::beast::http::verb::post, result_json);
}

void ExchangeObj::DeleteUrl(const string& path, string &result_json)
{
    GetUrlWithHeader(path, boost::beast::http::verb::delete_, result_json);
}

void ExchangeObj::GetUrlWithHeader(const string &path, boost::beast::http::verb action, string &str_result)
{
    try {
        boost::asio::io_context ioc;
        boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv11_client);
        load_root_certificates(ctx);
        ctx.set_verify_mode(boost::asio::ssl::verify_peer);
        boost::asio::ip::tcp::resolver resolver(ioc);
        boost::beast::ssl_stream<boost::beast::tcp_stream> stream(ioc, ctx);

        if (!SSL_set_tlsext_host_name(stream.native_handle(), HOST.c_str())) {
            boost::beast::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            throw boost::beast::system_error{ec};
        }

        auto const results = resolver.resolve(HOST.c_str(), PORT.c_str());
        boost::beast::get_lowest_layer(stream).connect(results);
        stream.handshake(boost::asio::ssl::stream_base::client);

        boost::beast::http::request<boost::beast::http::string_body> req {action, path, 10};
        req.set(boost::beast::http::field::host, HOST.c_str());
        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        if (!ApiKey_.empty())
            req.set("X-MBX-APIKEY", ApiKey_);
        req.prepare_payload();

        boost::beast::http::write(stream, req);
        boost::beast::flat_buffer buffer;
        boost::beast::http::response<boost::beast::http::dynamic_body> res;
        boost::beast::http::read(stream, buffer, res);
        str_result = boost::beast::buffers_to_string(res.body().cdata());

        boost::beast::error_code ec;
        stream.shutdown(ec);
        if(ec == boost::asio::error::eof) {
            ec = {};
        }

        if(ec)
            throw boost::beast::system_error{ec};

    } catch(std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
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

bool ExchangeObj::IsError(const boost::json::value &result)
{
    if (result.is_array())
        return false;

    if (result.is_null()) {
        ErrorMessage("<Error> Json Object is null !!!!");
        return true;
    }

    try {
        int code = result.at("code").to_number<int>();
        ErrorMessage((F("Error> Code: ") % code).str());
        return true;
    } catch (...) {
        return false;
    }

    return false;
}

void ExchangeObj::ErrorMessage(const std::string &msg)
{
    Log(BaseLogger::Level::Critical, msg);
}

void ExchangeObj::Log(BaseLogger::Level lv, const std::string &msg)
{
    if (Logger_ == nullptr)
        return;

    Logger_->Log(lv, msg);
}

string ExchangeObj::TimeFrame2String(TimeFrame tf)
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
