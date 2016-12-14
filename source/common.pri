CONFIG += c++14

CONFIG(debug,debug|release) {
    DEFINES += _DEBUG
}

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
    QMAKE_CXXFLAGS += -Wlogical-op -Wstrict-null-sentinel
}

win32 {
    # Disable silly inherits via dominance warning
    QMAKE_CXXFLAGS += -wd4250

    # Debug symbols
    QMAKE_CXXFLAGS_RELEASE += -Zi
    QMAKE_LFLAGS_RELEASE += /MAP /debug /opt:ref
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
    "VERSION=\"\\\"$$_VERSION\\\"\"" \
    "COPYRIGHT=\"\\\"$$_COPYRIGHT\\\"\""

INCLUDEPATH += $$PWD
