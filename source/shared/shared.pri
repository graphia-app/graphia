HEADERS += \
    $$PWD/attributes/iattribute.h \
    $$PWD/attributes/iattributerange.h \
    $$PWD/attributes/valuetype.h \
    $$PWD/commands/command.h \
    $$PWD/commands/icommand.h \
    $$PWD/commands/icommandmanager.h \
    $$PWD/graph/elementid.h \
    $$PWD/graph/elementtype.h \
    $$PWD/graph/grapharray.h \
    $$PWD/graph/igrapharrayclient.h \
    $$PWD/graph/igrapharray.h \
    $$PWD/graph/igraphcomponent.h \
    $$PWD/graph/igraph.h \
    $$PWD/graph/igraphmodel.h \
    $$PWD/graph/imutablegraph.h \
    $$PWD/loading/baseparser.h \
    $$PWD/loading/gmlfileparser.h \
    $$PWD/loading/graphmlparser.h \
    $$PWD/loading/iparser.h \
    $$PWD/loading/iparserthread.h \
    $$PWD/loading/iurltypes.h \
    $$PWD/loading/pairwisetxtfileparser.h \
    $$PWD/loading/tabulardata.h \
    $$PWD/loading/urltypes.h \
    $$PWD/plugins/basegenericplugin.h \
    $$PWD/plugins/baseplugin.h \
    $$PWD/plugins/iplugin.h \
    $$PWD/plugins/nodeattributetablemodel.h \
    $$PWD/plugins/userdata.h \
    $$PWD/plugins/userdatavector.h \
    $$PWD/plugins/usernodedata.h \
    $$PWD/ui/iselectionmanager.h \
    $$PWD/ui/visualisations/elementvisual.h \
    $$PWD/utils/circularbuffer.h \
    $$PWD/utils/deferredexecutor.h \
    $$PWD/utils/enumreflection.h \
    $$PWD/utils/fixedsizestack.h \
    $$PWD/utils/flags.h \
    $$PWD/utils/function_traits.h \
    $$PWD/utils/iterator_range.h \
    $$PWD/utils/movablepointer.h \
    $$PWD/utils/pair_iterator.h \
    $$PWD/utils/performancecounter.h \
    $$PWD/utils/preferences.h \
    $$PWD/utils/qmlenum.h \
    $$PWD/utils/scope_exit.h \
    $$PWD/utils/semaphore.h \
    $$PWD/utils/singleton.h \
    $$PWD/utils/threadpool.h \
    $$PWD/utils/utils.h

SOURCES += \
    $$PWD/graph/elementtype.cpp \
    $$PWD/loading/gmlfileparser.cpp \
    $$PWD/loading/graphmlparser.cpp \
    $$PWD/loading/pairwisetxtfileparser.cpp \
    $$PWD/loading/tabulardata.cpp \
    $$PWD/loading/urltypes.cpp \
    $$PWD/plugins/basegenericplugin.cpp \
    $$PWD/plugins/nodeattributetablemodel.cpp \
    $$PWD/plugins/userdata.cpp \
    $$PWD/plugins/userdatavector.cpp \
    $$PWD/plugins/usernodedata.cpp \
    $$PWD/utils/deferredexecutor.cpp \
    $$PWD/utils/performancecounter.cpp \
    $$PWD/utils/preferences.cpp \
    $$PWD/utils/semaphore.cpp \
    $$PWD/utils/threadpool.cpp \
    $$PWD/utils/utils.cpp

RESOURCES += \
    $$PWD/ui/shared.qrc

DISTFILES +=
