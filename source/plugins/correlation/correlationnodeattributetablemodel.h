#ifndef CORRELATIONNODEATTRIBUTETABLEMODEL_H
#define CORRELATIONNODEATTRIBUTETABLEMODEL_H

#include "shared/plugins/nodeattributetablemodel.h"

#include <QString>
#include <QObject>

#include <vector>
#include <map>

class CorrelationNodeAttributeTableModel : public NodeAttributeTableModel
{
    Q_OBJECT

private:
    std::vector<QString>* _dataColumnNames = nullptr;
    std::vector<double>* _dataValues = nullptr;

    // For fast lookup in dataValue(...)
    std::map<QString, size_t> _dataColumnIndexes;

    QStringList columnNames() const override;

public:
    void addDataColumns(std::vector<QString>* dataColumnNames = nullptr,
        std::vector<double>* dataValues = nullptr);

    QVariant dataValue(size_t row, const QString& columnName) const override;

    bool columnIsCalculated(const QString& columnName) const override;
    bool columnIsHiddenByDefault(const QString& columnName) const override;
    bool columnIsFloatingPoint(const QString& columnName) const override;
};

#endif // CORRELATIONNODEATTRIBUTETABLEMODEL_H
