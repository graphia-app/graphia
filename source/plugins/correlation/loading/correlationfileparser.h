#ifndef CORRELATIONFILEPARSER_H
#define CORRELATIONFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/tabulardata.h"
#include "datarecttablemodel.h"
#include <QString>
#include <QRect>
#include <QtConcurrent/QtConcurrent>

class CorrelationPluginInstance;

class CorrelationFileParser : public IParser
{
private:
    CorrelationPluginInstance* _plugin;
    QString _urlTypeName;
    QRect _dataRect;

public:
    explicit CorrelationFileParser(CorrelationPluginInstance* plugin, QString urlTypeName, QRect dataRect);
    bool parse(const QUrl& url, IGraphModel& graphModel, const ProgressFn& progressFn) override;
};

class CorrelationPreParser : public QObject
{
private:
    Q_OBJECT
    Q_PROPERTY(QString fileType MEMBER _fileType NOTIFY fileTypeChanged)
    Q_PROPERTY(QString fileUrl MEMBER _fileUrl NOTIFY fileUrlChanged)
    Q_PROPERTY(QRect dataRect MEMBER _dataRect NOTIFY dataRectChanged)
    Q_PROPERTY(QAbstractTableModel* model READ tableModel NOTIFY dataRectChanged)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(bool transposed READ transposed WRITE setTransposed NOTIFY transposeChanged)

    QFutureWatcher<void> _autoDetectDataRectangleWatcher;
    QFutureWatcher<void> _dataParserWatcher;
    QString _fileType;
    QString _fileUrl;
    QRect _dataRect;
    TabularData _data;
    DataRectTableModel _model;
    bool _transposed = false;

public:
    CorrelationPreParser();
    Q_INVOKABLE bool parse();
    Q_INVOKABLE void autoDetectDataRectangle(size_t column = 0, size_t row = 0);

    DataRectTableModel* tableModel();
    bool isRunning() { return _autoDetectDataRectangleWatcher.isRunning() || _dataParserWatcher.isRunning(); }

    bool transposed() const;
    void setTransposed(bool transposed);

signals:
    void dataRectChanged();
    void isRunningChanged();
    void fileUrlChanged();
    void fileTypeChanged();
    void dataLoaded();
    void transposeChanged();
public slots:
    void onDataParsed();
};

#endif // CORRELATIONFILEPARSER_H
