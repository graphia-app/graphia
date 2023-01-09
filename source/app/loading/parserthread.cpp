/* Copyright © 2013-2023 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "parserthread.h"

#include "graph/graphmodel.h"
#include "graph/mutablegraph.h"

#include "shared/utils/thread.h"

#include <atomic>

#include <QDebug>

ParserThread::ParserThread(GraphModel& graphModel, const QUrl& url) :
    _graphModel(&graphModel),
    _url(url)
{}

ParserThread::~ParserThread()
{
    cancel();
    wait();
}

void ParserThread::start(std::unique_ptr<IParser> parser)
{
    _cancelled = false;
    _parser = std::move(parser);
    _thread = std::thread(&ParserThread::run, this);
}

void ParserThread::cancel()
{
    if(_thread.joinable() && _parser != nullptr && !cancelled())
    {
        _parser->cancel();
        emit cancelledChanged();
    }
}

bool ParserThread::cancelled() const
{
    if(_parser != nullptr)
        return _parser->cancelled();

    return _cancelled;
}

void ParserThread::wait()
{
    if(_thread.joinable())
        _thread.join();
}

void ParserThread::reset()
{
    if(_parser != nullptr)
        _cancelled = _parser->cancelled();

    // Free up any memory used by the parser
    _parser = nullptr;
}

void ParserThread::run()
{
    u::setCurrentThreadName(QStringLiteral("Parser"));

    bool result = false;

    _graphModel->mutableGraph().performTransaction([this, &result](IMutableGraph& graph)
    {
        std::atomic<int> percentage;
        percentage = -1;

        _parser->setProgressFn([this, &percentage](int newPercentage)
        {
#ifdef _DEBUG
            if(newPercentage < -1 || newPercentage > 100)
                qDebug() << "progress called with unusual percentage" << newPercentage;
#endif
            if(newPercentage >= 0)
            {
                bool percentageIncreased = false;
                int expected = 0, desired = 0;

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

        result = _parser->parse(_url, _graphModel);

        if(!result)
        {
            // If the parsing failed, we shouldn't be wasting time updating a partially
            // constructed graph, so just clear it out
            graph.clear();

            // Also, we've already failed, so there is nothing else happening we should
            // be telling the user about
            graph.clearPhase();

            setFailureReason(_parser->failureReason());
        }

        // Extra processing may occur after the actual parsing, so we emit this here
        // in order that the progress indication doesn't just freeze
        emit progress(-1);
    });

    if(result)
        emit success(_parser.get());

    if(_parser->cancelled())
    {
        result = false;
        emit cancelledChanged();
    }

    emit complete(_url, result);
}
