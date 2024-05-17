include(${CMAKE_SOURCE_DIR}/source/common.cmake)

find_package(Qt6 COMPONENTS REQUIRED Quick Concurrent)

qt_add_qml_module(correlation_qml
    URI Graphia.Plugins.Correlation
    STATIC
    QML_FILES
        CorrelationParameters.qml
        ImportAnnotationsDialog.qml
        Main.qml
        PlotModalControls.qml
    SOURCES
        correlationplot.h correlationplot.cpp
            correlationplot_columnannotations.cpp
            correlationplot_continuous.cpp
            correlationplot_discrete.cpp
        correlationplotsaveimagecommand.h correlationplotsaveimagecommand.cpp
        correlationtabulardataparser.h correlationtabulardataparser.cpp
        correlationtype.h correlationtype.cpp
        graphsizeestimateplot.h graphsizeestimateplot.cpp
        importannotationskeydetection.h importannotationskeydetection.cpp
        qcpcolumnannotations.h qcpcolumnannotations.cpp
    RESOURCES
        plots.svg
)

target_link_libraries(correlation_qml
    PRIVATE Qt6::Quick Qt6::Concurrent
)
