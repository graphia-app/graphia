/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

    bool _cancelled = false;

public:
    ParserThread(GraphModel& graphModel, const QUrl& url);
    ~ParserThread() override;

    void start(std::unique_ptr<IParser> parser);
    void cancel();
    bool cancelled() const;

    void wait();

    void reset();

private:
    void run();

signals:
    void progress(int percentage);
    void success(IParser*);
    void complete(const QUrl& url, bool success);

    void cancelledChanged();
};

#endif // PARSERTHREAD_H
