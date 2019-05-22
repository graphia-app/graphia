#ifndef PARSERTHREAD_H
#define PARSERTHREAD_H

#include "shared/loading/iparserthread.h"
#include "shared/loading/iparser.h"
#include "shared/utils/failurereason.h"

#include <QObject>
#include <QUrl>

#include <thread>

class GraphModel;

class ParserThread : public QObject, public IParserThread, public FailureReason
{
    Q_OBJECT
private:
    GraphModel* _graphModel;
    QUrl _url;
    std::unique_ptr<IParser> _parser;
    std::thread _thread;

public:
    ParserThread(GraphModel& graphModel, QUrl url);
    ~ParserThread() override;

    void start(std::unique_ptr<IParser> parser);
    void cancel();
    bool cancelled() const;

    void wait();

    void reset();

private:
    void run();

signals:
    void progress(int percentage) const;
    void success(IParser*) const;
    void complete(const QUrl& url, bool success) const;

    void cancelledChanged() const;
};

#endif // PARSERTHREAD_H
