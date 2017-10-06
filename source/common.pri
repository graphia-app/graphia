CONFIG += c++14

CONFIG(debug,debug|release) {
    DEFINES += _DEBUG
}

# https://www.kdab.com/disabling-narrowing-conversions-in-signal-slot-connections/
DEFINES += QT_NO_NARROWING_CONVERSIONS_IN_CONNECT

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000 # disables all the APIs deprecated before Qt 6.0.0

gcc {
    QMAKE_CXXFLAGS += -Wpedantic -Wall -Wextra -Wcast-align -Wcast-qual \
        -Wdisabled-optimization -Wformat=2 -Winit-self \
        -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual \
        -Wnon-virtual-dtor -Wredundant-decls -Wshadow

    # Surprisingly, this actually makes a difference to the pearson correlation code
    QMAKE_CXXFLAGS += -funroll-loops

    # Debug symbols
    QMAKE_CXXFLAGS += -g
}

gcc:!clang {
    QMAKE_CXXFLAGS += -Wlogical-op -Wstrict-null-sentinel \
        -Wjump-misses-init -Wdouble-promotion
}

win32 {
    # Disable silly inherits via dominance warning
    QMAKE_CXXFLAGS += -wd4250

    # Debug symbols
    QMAKE_CXXFLAGS_RELEASE += -Zi
    QMAKE_LFLAGS_RELEASE += /MAP /debug /opt:ref

    QMAKE_EXTRA_COMPILERS += ml64
    ML_FLAGS = /c /nologo /D_M_X64 /W3 /Zi
    OTHER_FILES += $$MASM_SOURCES
    ml64.output = ${QMAKE_FILE_BASE}.obj
    ml64.commands = ml64 $$ML_FLAGS /Fo ${QMAKE_FILE_BASE}.obj ${QMAKE_FILE_NAME}
    ml64.input = MASM_SOURCES
}

mac {
    # Qt 5.9+ uses [[nodiscard]] which gets us a lot of warnings with mac clang
    QMAKE_CXXFLAGS += -Wno-c++1z-extensions
}

# OSX Info.plist
QMAKE_TARGET_BUNDLE_PREFIX = com.kajeka

_VERSION = $$(VERSION)
isEmpty(_VERSION) {
    _VERSION = "development"
}

_COPYRIGHT = $$(COPYRIGHT)
isEmpty(_COPYRIGHT) {
    _COPYRIGHT = "Copyright notice"
}

DEFINES += \
    "APP_URI=\"\\\"com.kajeka\\\"\"" \
    "APP_MINOR_VERSION=0" \
    "APP_MAJOR_VERSION=1" \
    "VERSION=\"\\\"$$_VERSION\\\"\"" \
    "COPYRIGHT=\"\\\"$$_COPYRIGHT\\\"\""

INCLUDEPATH += $$PWD
