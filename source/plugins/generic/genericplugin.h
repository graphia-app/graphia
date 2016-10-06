#ifndef GENERICPLUGIN_H
#define GENERICPLUGIN_H

#include "shared/plugins/basegenericplugin.h"

#include "shared/plugins/attribute.h"
#include "shared/plugins/attributestablemodel.h"

class GenericPluginInstance : public BaseGenericPluginInstance
{
    Q_OBJECT

    Q_PROPERTY(QAbstractTableModel* rowAttributes READ attributesTableModel CONSTANT)

public:
    GenericPluginInstance() :
        _attributesTableModel(&_rowAttributes)
    {}

private:
    Attributes _rowAttributes;

    AttributesTableModel _attributesTableModel;
    QAbstractTableModel* attributesTableModel() { return &_attributesTableModel; }
};

class GenericPlugin : public BaseGenericPlugin, public PluginInstanceProvider<GenericPluginInstance>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "genericplugin.json")

public:
    QString name() const { return "Generic"; }
    QString description() const
    {
        return tr("A plugin that loads generic graphs from a variety "
                  "of file formats.");
    }
    QString imageSource() const { return "qrc:///tools.svg"; }
    QString qmlPath() const { return "qrc:///qml/genericplugin.qml"; }
};

#endif // GENERICPLUGIN_H
