
TEMPLATE = lib
#DEFINES += LIBCRYPTOMARKET_LIBRARY

CONFIG += c++17

INCLUDEPATH += ./lib/openssl/
INCLUDEPATH += ./include/
INCLUDEPATH += ./lib/
INCLUDEPATH += C:\boost_1_80_0\

SOURCES += \
    src/binancefuturesexchange.cpp \
    src/libcryptomarket.cpp \
    src/binanceexchange.cpp \
    src/consolelogger.cpp \
    src/exchangeobj.cpp \
    src/symbol.cpp \
    src/websocket/basewebsocket.cpp \
    src/websocket/binancewebsocket.cpp \

HEADERS += \
    include/binanceexchange.h \
    include/binancefuturesexchange.h \
    include/exchangeobj.h \
    include/cert.h \
    include/websocket/basewebsocket.h \
    include/websocket/binancewebsocket.h \
    include/websocket/bybitwebsocket.h \
    libcryptomarket.h

LIBS += -lws2_32
LIBS += ./bin/objects/libcrypto.dll.a
LIBS += ./bin/objects//libssl.dll.a

DESTDIR = bin

unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
