TEMPLATE = lib
CONFIG += staticlib warn_off

TARGET = thirdparty

include(../common.pri)

include(qtsingleapplication/qtsingleapplication.pri)
include(breakpad/breakpad.pri)
include(SortFilterProxyModel/SortFilterProxyModel.pri)
