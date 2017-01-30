#ifndef PARSERTHREAD_H
#define PARSERTHREAD_H

#include "shared/loading/iparserthread.h"
#include "shared/loading/iparser.h"

#include <QObject>
#include <QUrl>

#include <thread>
#include <functional>

class MutableGraph;

class ParserThread : public QObject, public IParserThread
{
    Q_OBJECT
private:
    MutableGraph* _graph;
    QUrl _url;
    std::unique_ptr<IParser> _parser;
    std::thread _thread;
    std::function<void()> _loadSuccessFn;

public:
    ParserThread(MutableGraph& graph, const QUrl& url);
    virtual ~ParserThread();

    void start(std::unique_ptr<IParser> parser,
               std::function<void()> loadSuccessFn = []{});
    void cancel();

private:
    void run();

signals:
    void progress(int percentage) const;
    void complete(bool success) const;
};

#endif // PARSERTHREAD_H
