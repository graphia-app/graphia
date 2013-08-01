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
    graph/qgraph.cpp \
    parsers/gmlfileparser.cpp

HEADERS  += mainwindow.h \
    graph/graph.h \
    graph/qgraph.h \
    graph/grapharray.h \
    parsers/graphfileparser.h \
    parsers/gmlfileparser.h

FORMS    += mainwindow.ui
