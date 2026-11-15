QT += core gui quick quickcontrols2 testlib webenginequick
CONFIG += c++17 testcase
TARGET = livesheet_tests
TEMPLATE = app

INCLUDEPATH += ../src

HEADERS += ../src/backend.h ../src/theme.h
SOURCES += livesheet_tests.cpp ../src/backend.cpp ../src/theme.cpp
