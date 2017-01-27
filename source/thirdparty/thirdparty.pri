include(boost/boost.pri)

# Things that use QCustomPlot need these
DEFINES += QCUSTOMPLOT_USE_OPENGL
QT += printsupport

THIRDPARTY_LIB_DIR = $$top_builddir/thirdparty

win32:CONFIG(release, debug|release): LIBS += -L$$THIRDPARTY_LIB_DIR/release/ -lthirdparty -WHOLEARCHIVE:thirdparty
else:win32:CONFIG(debug, debug|release): LIBS += -L$$THIRDPARTY_LIB_DIR/debug/ -lthirdparty -WHOLEARCHIVE:thirdparty
else:mac: LIBS += -L$$THIRDPARTY_LIB_DIR -framework CoreFoundation -Wl,-force_load,$$THIRDPARTY_LIB_DIR/libthirdparty.a -lthirdparty
else:unix: LIBS += -L$$THIRDPARTY_LIB_DIR -Wl,-whole-archive -lthirdparty -Wl,-no-whole-archive
