QT += core
QT -= gui
QMAKE_CXXFLAGS += -std=gnu++1y
LIBS += -ldl

TARGET = ConsView
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    *.cpp

HEADERS += \
    *.h
