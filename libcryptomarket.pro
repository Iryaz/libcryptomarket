
TEMPLATE = lib
#DEFINES += LIBCRYPTOMARKET_LIBRARY

CONFIG += c++17

INCLUDEPATH += ./lib/curl/
INCLUDEPATH += ./lib/libwebsockets/
INCLUDEPATH += ./lib/openssl/
INCLUDEPATH += ./include/
INCLUDEPATH += ./lib/
INCLUDEPATH += C:\boost_1_80_0\

SOURCES += \
    src/libcryptomarket.cpp \
    src/binanceexchange.cpp \
    src/binancefuturesexchange.cpp \
    src/consolelogger.cpp \
    src/exchangeobj.cpp \
    src/symbol.cpp \
    src/websocket/basewebsocket.cpp \
    src/websocket/binancewebsocket.cpp \
    src/websocket/bybitwebsocket.cpp

HEADERS += \
    include/binanceexchange.h \
    include/binancefuturesexchange.h \
    include/exchangeobj.h \
    include/websocket/basewebsocket.h \
    include/websocket/binancewebsocket.h \
    include/websocket/bybitwebsocket.h \
    libcryptomarket.h

LIBS += -lws2_32
LIBS += ./bin/objects/libcrypto.dll.a
LIBS += ./bin/objects//libssl.dll.a
LIBS += ./bin/libcurl.dll

DESTDIR = bin

unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
