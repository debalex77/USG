QT += core sql
QT -= gui
CONFIG += console c++20
CONFIG -= app_bundle
TARGET = tst_uuidmigration
INCLUDEPATH += ../../src
SOURCES += tst_uuidmigration.cpp ../../src/database/uuidmigrationplan.cpp
