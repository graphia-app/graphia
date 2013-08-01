#-------------------------------------------------
#
# Project created by QtCreator 2013-07-31T14:34:23
#
#-------------------------------------------------

QMAKE_CXXFLAGS += -std=c++11

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GraphTool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    parsers/gmlfileparser.cpp \
    graph/graph.cpp

HEADERS  += mainwindow.h \
    graph/grapharray.h \
    parsers/graphfileparser.h \
    parsers/gmlfileparser.h \
    graph/graph.h

FORMS    += mainwindow.ui
