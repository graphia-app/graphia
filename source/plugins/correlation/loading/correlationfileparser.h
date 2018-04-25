#ifndef CORRELATIONFILEPARSER_H
#define CORRELATIONFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/tabulardata.h"
#include <QString>
#include <QRect>

class CorrelationPluginInstance;

class CorrelationFileParser : public IParser
{
private:
    CorrelationPluginInstance* _plugin;
    QString _urlTypeName;

public:
    explicit CorrelationFileParser(CorrelationPluginInstance* plugin, QString urlTypeName);

    bool parse(const QUrl& url, IGraphModel& graphModel, const ProgressFn& progressFn) override;
};

class CorrelationPreParser : public QObject
{
private:
    Q_OBJECT
    Q_PROPERTY(QString fileType MEMBER _fileType NOTIFY fileTypeChanged)
    Q_PROPERTY(QString fileUrl MEMBER _fileUrl NOTIFY fileUrlChanged)
    Q_PROPERTY(QRect dataRect MEMBER _dataRect NOTIFY dataRectChanged)
    Q_PROPERTY(int columnCount READ columnCount NOTIFY dataRectChanged)
    Q_PROPERTY(int rowCount READ rowCount NOTIFY dataRectChanged)

    QString _fileType;
    QString _fileUrl;
    QRect _dataRect;
    TabularData* _data = nullptr;

    int rowCount();
    int columnCount();
public:
    Q_INVOKABLE bool parse();
    Q_INVOKABLE QString dataAt(int column, int row);
signals:
    void dataRectChanged();
    void fileUrlChanged();
    void fileTypeChanged();
};

#endif // CORRELATIONFILEPARSER_H
