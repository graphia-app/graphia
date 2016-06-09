#ifndef PARSERTHREAD_H
#define PARSERTHREAD_H

#include "shared/loading/iparser.h"

#include <QObject>
#include <QUrl>

#include <thread>

class MutableGraph;

class ParserThread : public QObject
{
    Q_OBJECT
private:
    MutableGraph& _graph;
    QUrl _url;
    IParser* _parser;
    std::thread _thread;

public:
    ParserThread(MutableGraph& graph, const QUrl& url, IParser* parser);
    virtual ~ParserThread();

    void start();
    void cancel();

private:
    void run();

signals:
    void progress(int percentage) const;
    void complete(bool success) const;
};

#endif // PARSERTHREAD_H
