#ifndef CORRELATIONFILEPARSER_H
#define CORRELATIONFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/tabulardata.h"

#include "shared/utils/cancellable.h"

#include "datarecttablemodel.h"

#include <QString>
#include <QRect>
#include <QtConcurrent/QtConcurrent>

#include <memory>

class CorrelationPluginInstance;

class CorrelationFileParser : public IParser
{
private:
    CorrelationPluginInstance* _plugin;
    QString _urlTypeName;
    TabularData _tabularData;
    QRect _dataRect;

public:
    explicit CorrelationFileParser(CorrelationPluginInstance* plugin, QString urlTypeName, TabularData& tabularData, QRect dataRect);
    bool parse(const QUrl& url, IGraphModel* graphModel) override;
    static bool canLoad(const QUrl&) { return true; }
};

Q_DECLARE_METATYPE(std::shared_ptr<TabularData>)

class TabularDataParser : public QObject, public Cancellable
{
    Q_OBJECT

    Q_PROPERTY(QRect dataRect MEMBER _dataRect NOTIFY dataRectChanged)
    Q_PROPERTY(std::shared_ptr<TabularData> data MEMBER _dataPtr NOTIFY dataChanged)
    Q_PROPERTY(QAbstractTableModel* model READ tableModel NOTIFY dataRectChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool transposed READ transposed WRITE setTransposed NOTIFY transposeChanged)

    Q_PROPERTY(int progress MEMBER _progress WRITE setProgress NOTIFY progressChanged)
    Q_PROPERTY(bool complete MEMBER _complete NOTIFY completeChanged)

private:
    QFutureWatcher<void> _autoDetectDataRectangleWatcher;
    QFutureWatcher<void> _dataParserWatcher;
    QRect _dataRect;
    std::shared_ptr<TabularData> _dataPtr;
    DataRectTableModel _model;
    bool _transposed = false;

    int _progress = -1;
    bool _complete = false;

    void setProgress(int progress);

public:
    TabularDataParser();
    Q_INVOKABLE bool parse(const QUrl& fileUrl, const QString& fileType);
    Q_INVOKABLE void cancelParse() { cancel(); }
    Q_INVOKABLE void autoDetectDataRectangle(size_t column = 0, size_t row = 0);

    Q_INVOKABLE void clearData();

    DataRectTableModel* tableModel();
    bool busy() { return _autoDetectDataRectangleWatcher.isRunning() || _dataParserWatcher.isRunning(); }

    bool transposed() const;
    void setTransposed(bool transposed);

signals:
    void dataChanged();
    void dataRectChanged();
    void busyChanged();
    void fileUrlChanged();
    void fileTypeChanged();
    void dataLoaded();
    void transposeChanged();
    void progressChanged();
    void completeChanged();

private slots:
    void onDataLoaded();
};

#endif // CORRELATIONFILEPARSER_H
