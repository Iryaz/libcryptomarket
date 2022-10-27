﻿#include "websocket/basewebsocket.h"

#include <boost/asio.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/signals2.hpp>
#include <boost/json.hpp>
#include <libcryptomarket.h>
#include <thread>

namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

BaseWebSocket::BaseWebSocket(const std::string& symbol, int subscribe_flags)
{
    SubscribeFlags = subscribe_flags;
    IsStart_ = true;
    Thread_Ptr = nullptr;
    SetSymbol(symbol);
    Context_ = nullptr;

    UpdateMarketDepthCallback_ = nullptr;
    AddTradeCallback_ = nullptr;
    UpdateCandleCallback_ = nullptr;
}

BaseWebSocket::~BaseWebSocket()
{
    if (Thread_Ptr != nullptr)
        delete Thread_Ptr;
}

void BaseWebSocket::SetPort(int port)
{
    Port = std::to_string(port);
}

void BaseWebSocket::SetHost(const std::string& exchange_host)
{
    Host = exchange_host;
}

void BaseWebSocket::Stop()
{
    if (IsStart_ == false)
        return;
    if (Thread_Ptr != nullptr) {
        if (Thread_Ptr->joinable())
            Thread_Ptr->join();
        delete Thread_Ptr;
    }

    IsStart_ = false;
}

void BaseWebSocket::SetPath(const std::string& path)
{
    Path_ = path;
}

void BaseWebSocket::StartThread()
{
    IsStart_ = true;
    Thread_Ptr = new std::thread(std::bind(&BaseWebSocket::StartLoop, this));
}

bool BaseWebSocket::StartLoop()
{
    try {
        boost::asio::io_context io_ctx;
        boost::asio::ssl::context ssl_ctx {boost::asio::ssl::context::tlsv12_client};
        tcp::resolver resolver(io_ctx);
        websocket::stream<beast::ssl_stream<tcp::socket> > ws(io_ctx, ssl_ctx);

        auto const results = resolver.resolve(Host, Port);
        net::connect(ws.next_layer().next_layer(), results.begin(), results.end());
        ws.next_layer().handshake(boost::asio::ssl::stream_base::client);
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req) {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                    "libcryptomarket");
                }));

        ws.handshake(Host, Path_);

        if (!Message_.empty())
            ws.write(net::buffer(Message_));

        beast::flat_buffer buffer;
        while (IsStart_) {
            ws.read(buffer);
            if (buffer.size() > 0) {
                ParseBuffer(buffer);
            }
            buffer.clear();
        }
        //std::cout << "Everything finished fine" << std::endl;
    } catch (std::exception const& e) {
        //std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }

    return true;
}

void BaseWebSocket::ParseBuffer(beast::flat_buffer buffer)
{
    json::stream_parser parser;
    parser.write(beast::buffers_to_string(buffer.data()));
    try {
        auto val = parser.release();
        ParseJSon(val);
    } catch (std::exception& e) {

    }
}

TimeFrame BaseWebSocket::GetTimeFrame(const std::string& tf)
{
    if (tf == "1m") return TimeFrame_1m;
    if (tf == "5m") return TimeFrame_5m;
    if (tf == "15m") return TimeFrame_15m;
    if (tf == "1h") return TimeFrame_1h;
    if (tf == "4h") return TimeFrame_4h;
    if (tf == "1d") return TimeFrame_1d;

    return TimeFrame_1m;
}