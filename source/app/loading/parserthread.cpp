#include "parserthread.h"

#include "graph/graphmodel.h"
#include "graph/mutablegraph.h"

#include "shared/utils/thread.h"

#include <atomic>

#include <QDebug>

ParserThread::ParserThread(GraphModel& graphModel, QUrl url) :
    _graphModel(&graphModel),
    _url(std::move(url))
{}

ParserThread::~ParserThread()
{
    cancel();

    if(_thread.joinable())
        _thread.join();
}

void ParserThread::start(std::unique_ptr<IParser> parser)
{
    _parser = std::move(parser);
    _thread = std::thread(&ParserThread::run, this);
}

void ParserThread::cancel()
{
    if(_thread.joinable() && !_parser->cancelled())
    {
        _parser->cancel();
        emit cancelledChanged();
    }
}

bool ParserThread::cancelled() const
{
    return _parser->cancelled();
}

void ParserThread::run()
{
    u::setCurrentThreadName(QStringLiteral("Parser"));

    bool result = false;

    _graphModel->mutableGraph().performTransaction(
        [this, &result](IMutableGraph& graph)
        {
            std::atomic<int> percentage;
            percentage = -1;

            result = _parser->parse(_url, *_graphModel,
                [this, &percentage](int newPercentage)
                {
#ifdef _DEBUG
                    if(newPercentage < -1 || newPercentage > 100)
                        qDebug() << "progress called with unusual percentage" << newPercentage;
#endif
                    if(newPercentage >= 0)
                    {
                        bool percentageIncreased = false;
                        int expected, desired;

                        do
                        {
                            expected = percentage.load();
                            desired = newPercentage > expected ? newPercentage : expected;
                            percentageIncreased = desired != expected;
                        }
                        while(!percentage.compare_exchange_weak(expected, desired));

                        if(percentageIncreased)
                            emit progress(newPercentage);
                    }
                    else
                    {
                        percentage = newPercentage;
                        emit progress(newPercentage);
                    }
                });

            if(!result)
            {
                // If the parsing failed, we shouldn't be wasting time updating a partially
                // constructed graph, so just clear it out
                graph.clear();

                // Also, we've already failed, so there is nothing else happening we should
                // be telling the user about
                graph.clearPhase();
            }

            // Extra processing may occur after the actual parsing, so we emit this here
            // in order that the progress indication doesn't just freeze
            emit progress(-1);
        });

    if(result)
        emit success(_parser.get());
    else if(_parser->cancelled())
        emit cancelledChanged();

    emit complete(_url, result);
}
