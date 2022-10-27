
TARGET = binance
TEMPLATE = app

CONFIG += c++11

SOURCES += \
    main.cpp

INCLUDEPATH += ../../

LIBS += ../../bin/libcryptomarket.dll
DESTDIR += ../../bin
