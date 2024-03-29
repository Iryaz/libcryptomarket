﻿#ifndef BASEWEBSOCKET_H
#define BASEWEBSOCKET_H

#include <boost/beast/core.hpp>
#include <boost/json.hpp>
#include <libcryptomarket.h>
#include <string>
#include <thread>

namespace json = boost::json;
namespace beast = boost::beast;
using namespace libcryptomarket;

class BaseWebSocket
{
public:
    enum Type {
        Spot,
        Futures
    };

    BaseWebSocket(Type type, const std::string& symbol, int subscribe_flags);
    virtual ~BaseWebSocket();

    int GetSubscribeFlags()                             { return SubscribeFlags;    }
    void SetContext(void* context)                      { Context_ = context;       }
    void *GetContext()                                  { return Context_;          }
    const std::string GetSymbol()                       { return Symbol_;           }
    const std::string GetExchange()                     { return Exchange_;         }
    virtual void SetSymbol(const std::string& symbol)   { Symbol_ = symbol;         }
    virtual void SetPath(const std::string& path);
    virtual void SetWebSocketPort(int port);
    void SetHost(const std::string& exchange_host);
    void StartThread();
    void Stop();
    void SetLogger(BaseLogger* logger);

    void SetMarketDepthEvent(libcryptomarket::UpdateMarketDepthEvent event) { UpdateMarketDepthCallback_ = event;   }
    void SetAddTradeEvent(libcryptomarket::AddTradeEvent event)             { AddTradeCallback_ = event;            }
    void SetUpdateCandleEvent(libcryptomarket::UpdateCandleEvent event)     { UpdateCandleCallback_ = event;        }
    void SetUpdateBalanceEvent(libcryptomarket::UpdateBalanceEvent event)   { UpdateBalanceCallback_ = event;       }
    void SetUpdatePositionEvent(libcryptomarket::UpdatePositionEvent event) { UpdatePositionCallback_ = event;      }
    void SetUpdateOrderEvent(libcryptomarket::UpdateOrderEvent event)       { UpdateOrderCallback_ = event;         }
    void SetUpdateMarkPrice(libcryptomarket::UpdateMarkPriceEvent event)    { UpdateMarkPriceCallback_ = event;     }

protected:
    typedef enum {
        UNKNOWN = 0,
        DEPTH_UPDATE = 1,
        AGG_TRADE = 2,
        KLINE = 3,
        MARK_PRICE = 4,
        MARGIN_CALL = 5,
        ACCOUNT_UPDATE = 6,
        ORDER_TRADE_UPDATE = 7
    } DataEventType;

    void* Context_;
    std::string Exchange_;
    std::string ClientName_;
    std::string Host;
    std::string Port;
    std::string Path_;
    std::string Symbol_;
    std::string Message_;
    Type Type_;

    virtual bool StartLoop();
    virtual void ParseJSon(boost::json::value& val) = 0;
    virtual void Init(int subscribe_flag, const std::string& listen_key = "") = 0;
    TimeFrame GetTimeFrame(const std::string& tf);

    UpdateMarketDepthEvent UpdateMarketDepthCallback_;
    AddTradeEvent AddTradeCallback_;
    UpdateCandleEvent UpdateCandleCallback_;
    UpdateBalanceEvent UpdateBalanceCallback_;
    UpdatePositionEvent UpdatePositionCallback_;
    UpdateOrderEvent UpdateOrderCallback_;
    UpdateMarkPriceEvent UpdateMarkPriceCallback_;

    void ParseBuffer(beast::flat_buffer buffer);
    bool IsStart_;
    BaseLogger *Logger;

    void ErrorMessage(const string& message);
    void InfoMessage(const string& message);
    void WarningMessage(const string& message);

    int GetSubcribeFlags() { return SubscribeFlags; }

private:
    std::thread *Thread_Ptr;;
    int SubscribeFlags;
};

#endif // BASEWEBSOCKET_H
