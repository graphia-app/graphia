CONFIG += c++14

CONFIG(debug,debug|release) {
    DEFINES += _DEBUG
}

gcc {
    QMAKE_CXXFLAGS += -Wpedantic -Wall -Wextra -Wcast-align -Wcast-qual \
        -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations \
        -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual \
        -Wnon-virtual-dtor -Wredundant-decls -Wshadow -Wundef
}

gcc:!clang {
    QMAKE_CXXFLAGS += -Wlogical-op -Wstrict-null-sentinel
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
