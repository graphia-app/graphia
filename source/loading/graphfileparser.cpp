#include "graphfileparser.h"

#include "../graph/mutablegraph.h"

#include "../utils/utils.h"

GraphFileParserThread::GraphFileParserThread(MutableGraph& graph,
                                             std::unique_ptr<GraphFileParser> graphFileParser) :
    _graph(graph),
    _graphFileParser(std::move(graphFileParser))
{}

GraphFileParserThread::~GraphFileParserThread()
{
    cancel();

    if(_thread.joinable())
        _thread.join();
}

void GraphFileParserThread::start()
{
    _thread = std::thread(&GraphFileParserThread::run, this);
}

void GraphFileParserThread::cancel()
{
    if(_thread.joinable())
        _graphFileParser->cancel();
}

void GraphFileParserThread::run()
{
    u::setCurrentThreadName("Parser");

    connect(_graphFileParser.get(), &GraphFileParser::progress, this, &GraphFileParserThread::progress);

    bool result;

    _graph.performTransaction(
        [this, &result](MutableGraph& graph)
        {
            result = _graphFileParser->parse(graph);

            // Extra processing may occur after the actual parsing, so we emit this here
            // in order that the progress indication doesn't just freeze
            emit progress(-1);
        });

    emit complete(result);
}
