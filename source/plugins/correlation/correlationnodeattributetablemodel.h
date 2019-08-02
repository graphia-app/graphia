#ifndef CORRELATIONNODEATTRIBUTETABLEMODEL_H
#define CORRELATIONNODEATTRIBUTETABLEMODEL_H

#include "shared/plugins/nodeattributetablemodel.h"

#include <QString>
#include <QObject>

#include <vector>

class CorrelationNodeAttributeTableModel : public NodeAttributeTableModel
{
    Q_OBJECT

private:
    std::vector<QString>* _dataColumnNames =  nullptr;
    std::vector<double>* _dataValues = nullptr;

    int _firstDataColumnRole = -1;

    QStringList columnNames() const override;

public:
    void initialise(IDocument* document, UserNodeData* userNodeData,
                    std::vector<QString>* dataColumnNames = nullptr,
                    std::vector<double>* dataValues = nullptr);

    bool columnIsCalculated(const QString& columnName) const override;
    bool columnIsHiddenByDefault(const QString& columnName) const override;
    bool columnIsFloatingPoint(const QString& columnName) const override;
};

#endif // CORRELATIONNODEATTRIBUTETABLEMODEL_H
