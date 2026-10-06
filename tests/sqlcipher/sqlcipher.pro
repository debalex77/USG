QT += core sql
CONFIG += console c++20
CONFIG -= app_bundle
TEMPLATE = app
TARGET = tst_sqlcipher
INCLUDEPATH += ../../src
SOURCES += tst_sqlcipher.cpp ../../src/common/maindatabaseconnectioncontext.cpp
