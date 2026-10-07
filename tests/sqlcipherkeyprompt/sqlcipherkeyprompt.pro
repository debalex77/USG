QT += core gui widgets sql
CONFIG += console c++20
CONFIG -= app_bundle
TEMPLATE = app
TARGET = tst_sqlcipherkeyprompt
INCLUDEPATH += ../../src
SOURCES += tst_sqlcipherkeyprompt.cpp ../../src/common/maindatabaseconnectioncontext.cpp

SOURCES += ../../src/ui/widgets/lineeditpassword.cpp
HEADERS += ../../src/ui/widgets/lineeditpassword.h
