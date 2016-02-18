#ifndef GRAPHFILEPARSER_H
#define GRAPHFILEPARSER_H

#include <QObject>

#ifndef Q_MOC_RUN
#include <iostream>
#endif

#include <memory>
#include <iterator>
#include <cstdint>
#include <thread>
#include <atomic>

class MutableGraph;

//FIXME rename this FileParser? files not necessarily graphs
class GraphFileParser : public QObject
{
    Q_OBJECT
signals:
    void progress(int percentage) const;

public:
    virtual bool parse(MutableGraph& graph) = 0;

private:
    std::atomic<bool> _cancelAtomic;
    void setCancel(bool cancel)
    {
        _cancelAtomic = cancel;
    }

public:
    void cancel()
    {
        setCancel(true);
    }

    bool cancelled()
    {
        return _cancelAtomic;
    }
};

class GraphFileParserThread : public QObject
{
    Q_OBJECT
private:
    MutableGraph& _graph;
    std::unique_ptr<GraphFileParser> _graphFileParser;
    std::thread _thread;

public:
    GraphFileParserThread(MutableGraph& graph, std::unique_ptr<GraphFileParser> graphFileParser);
    virtual ~GraphFileParserThread();

    void start();
    void cancel();

private:
    void run();

signals:
    void progress(int percentage) const;
    void complete(bool success) const;
};

#endif // GRAPHFILEPARSER_H
