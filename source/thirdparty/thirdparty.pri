include(boost/boost.pri)

# Things that use QCustomPlot need this define
DEFINES += QCUSTOMPLOT_USE_OPENGL

THIRDPARTY_LIB_DIR = $$top_builddir/thirdparty

win32:CONFIG(release, debug|release): LIBS += -L$$THIRDPARTY_LIB_DIR/release/ -lthirdparty
else:win32:CONFIG(debug, debug|release): LIBS += -L$$THIRDPARTY_LIB_DIR/debug/ -lthirdparty
else:unix: LIBS += -L$$THIRDPARTY_LIB_DIR -lthirdparty
