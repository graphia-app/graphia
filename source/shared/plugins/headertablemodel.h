#ifndef HEADERTABLEMODEL_H
#define HEADERTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

class HeaderTableModel : public QAbstractTableModel
{
    Q_PROPERTY(int totalHeaderCount READ totalHeaderCount NOTIFY totalCount)
private:

public:
    HeaderTableModel();
};

#endif // HEADERTABLEMODEL_H
