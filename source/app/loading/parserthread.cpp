#include "parserthread.h"

#include "../graph/mutablegraph.h"

#include "shared/utils/utils.h"

ParserThread::ParserThread(MutableGraph& graph, const QUrl& url,
                           std::unique_ptr<IParser> parser) :
    _graph(graph),
    _url(url),
    _parser(std::move(parser))
{}

ParserThread::~ParserThread()
{
    cancel();

    if(_thread.joinable())
        _thread.join();
}

void ParserThread::start()
{
    _thread = std::thread(&ParserThread::run, this);
}

void ParserThread::cancel()
{
    if(_thread.joinable())
        _parser->cancel();
}

void ParserThread::run()
{
    u::setCurrentThreadName("Parser");

    bool result = false;

    _graph.performTransaction(
        [this, &result](MutableGraph& graph)
        {
            int newPercentage = -1;

            result = _parser->parse(_url, graph,
                [this, &newPercentage](int percentage)
                {
                    if(percentage <= 0 || percentage > newPercentage)
                    {
                        newPercentage = percentage;
                        emit progress(percentage);
                    }
                });

            if(!result)
            {
                // If the parsing failed, we shouldn't be wasting time updating a partially
                // constructed graph, so just clear it out
                graph.clear();
            }

            // Extra processing may occur after the actual parsing, so we emit this here
            // in order that the progress indication doesn't just freeze
            emit progress(-1);
        });

    emit complete(result);
}
