
TARGET = binance-test
TEMPLATE = app

CONFIG += c++11

SOURCES += \
    binance-test.cpp

INCLUDEPATH += ../../

LIBS += ../../bin/libcryptomarket.dll
DESTDIR += ../../bin
