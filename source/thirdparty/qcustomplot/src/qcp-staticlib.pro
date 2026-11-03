#
# Project to compile QCustomPlot as static library (.a/.lib) from the non-amalgamated sources
#
# It creates two static library files: qcustomplot and qcustomplotd, for release and debug mode respectively.
# If you want to be able to step into QCustomPlot code in your debug-mode-application, link with qcustomplotd.
# If compiling your application in release mode, you should link with qcustomplot.
#
# In your project's HEADERS, you may use either the amalgamated header qcustomplot.h or all separated headers.
# (note that qmake understands "*.h" if you choose the latter option.)
#

QT += core gui# opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++11
lessThan(QT_MAJOR_VERSION, 5): QMAKE_CXXFLAGS += -std=c++11

TEMPLATE = lib
CONFIG += qt staticlib debug_and_release build_all
DESTDIR = $$PWD

QMAKE_CXXFLAGS += -Wall -Wextra
QMAKE_CXXFLAGS += -Wold-style-cast -Wlogical-op -Wduplicated-branches -Wduplicated-cond
#QMAKE_CXXFLAGS += -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Woverloaded-virtual -Wredundant-decls -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused
#QMAKE_CXXFLAGS += -Werror

#DEFINES += QCUSTOMPLOT_USE_OPENGL

VERSION = 2.1.1
TARGET = qcustomplot
CONFIG(debug, debug|release) {
  TARGET = $$join(TARGET,,,d) # if compiling in debug mode, append a "d" to the library name
  MOC_DIR = build-debug
  OBJECTS_DIR = build-debug
} else {
  MOC_DIR = build-release
  OBJECTS_DIR = build-release
}

HEADERS += \
    global.h \
    painter.h \
    paintbuffer.h \
    layer.h \
    axis/range.h \
    axis/labelpainter.h \
    axis/axis.h \
    axis/axisticker.h \
    plottable.h \
    item.h \
    lineending.h \
    core.h \
    layout.h \
    plottables/plottable-graph.h \
    plottables/plottable-curve.h \
    plottables/plottable-bars.h \
    plottables/plottable-statisticalbox.h \
    plottables/plottable-colormap.h \
    plottables/plottable-financial.h \
    plottables/plottable-errorbar.h \
    items/item-straightline.h \
    items/item-line.h \
    items/item-curve.h \
    items/item-rect.h \
    items/item-text.h \
    items/item-ellipse.h \
    items/item-pixmap.h \
    items/item-tracer.h \
    items/item-bracket.h \
    layoutelements/layoutelement-axisrect.h \
    layoutelements/layoutelement-legend.h \
    layoutelements/layoutelement-textelement.h \
    layoutelements/layoutelement-colorscale.h \
    colorgradient.h \
    vector2d.h \
    axis/axistickerdatetime.h \
    axis/axistickertime.h \
    axis/axistickerfixed.h \
    axis/axistickertext.h \
    axis/axistickerpi.h \
    axis/axistickerlog.h \
    datacontainer.h \
    selection.h \
    selectionrect.h \
    plottable1d.h \
    scatterstyle.h \
    selectiondecorator-bracket.h \
    polar/radialaxis.h \
    polar/layoutelement-angularaxis.h \
    polar/polargrid.h \
    polar/polargraph.h

SOURCES += \
    painter.cpp \
    paintbuffer.cpp \
    layer.cpp \
    axis/range.cpp \
    axis/labelpainter.cpp \
    axis/axis.cpp \
    axis/axisticker.cpp \
    plottable.cpp \
    item.cpp \
    lineending.cpp \
    core.cpp \
    layout.cpp \
    plottables/plottable-graph.cpp \
    plottables/plottable-curve.cpp \
    plottables/plottable-bars.cpp \
    plottables/plottable-statisticalbox.cpp \
    plottables/plottable-colormap.cpp \
    plottables/plottable-financial.cpp \
    plottables/plottable-errorbar.cpp \
    items/item-straightline.cpp \
    items/item-line.cpp \
    items/item-curve.cpp \
    items/item-rect.cpp \
    items/item-text.cpp \
    items/item-ellipse.cpp \
    items/item-pixmap.cpp \
    items/item-tracer.cpp \
    items/item-bracket.cpp \
    layoutelements/layoutelement-axisrect.cpp \
    layoutelements/layoutelement-legend.cpp \
    layoutelements/layoutelement-textelement.cpp \
    layoutelements/layoutelement-colorscale.cpp \
    colorgradient.cpp \
    vector2d.cpp \
    axis/axistickerdatetime.cpp \
    axis/axistickertime.cpp \
    axis/axistickerfixed.cpp \
    axis/axistickertext.cpp \
    axis/axistickerpi.cpp \
    axis/axistickerlog.cpp \
    selection.cpp \
    selectionrect.cpp \
    scatterstyle.cpp \
    selectiondecorator-bracket.cpp \
    polar/radialaxis.cpp \
    polar/layoutelement-angularaxis.cpp \
    polar/polargrid.cpp \
    polar/polargraph.cpp

OTHER_FILES += \
    ../changelog.txt \
    doc-classoverview.dox \
    doc-layoutsystem.dox \
    doc-mainpage.dox \
    doc-specialqtflags.dox \
    doc-performance.dox \
    doc-dataselection.dox
