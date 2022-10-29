
TARGET = bybit-test
TEMPLATE = app

CONFIG += c++11

SOURCES += \
    bybit-test.cpp

INCLUDEPATH += ../../

LIBS += ../../bin/libcryptomarket.dll
DESTDIR += ../../bin
