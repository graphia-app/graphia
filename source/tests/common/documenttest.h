/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef DOCUMENTTEST_H
#define DOCUMENTTEST_H

#include "app/application.h"
#include "app/graph/graph.h"
#include "app/graph/graphmodel.h"
#include "app/graph/mutablegraph.h"
#include "app/ui/document.h"
#include "app/ui/qml/Graphia/graphdisplay.h"
#include "app/ui/selectionmanager.h"

#include <QFile>
#include <QMetaObject>
#include <QObject>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
#include <QVariant>
#include <QVariantMap>

#include <memory>

// A base for tests of a Document, as the UI uses it, but with a GraphDisplay that isn't
// in a window, so there's no rendering. Files are loaded with the real plugins, which
// are in the build directory, given by GRAPHIA_BUILD_DIR
class DocumentTest : public QObject // clazy:exclude=missing-qobject-macro
{
protected:
    static constexpr int Timeout = 60000;

    QTemporaryDir _dir;
    std::unique_ptr<Application> _application;
    std::unique_ptr<GraphDisplay> _graphDisplay;
    std::unique_ptr<Document> _document;

    // Call from initTestCase, once the TestEnvironment is initialised
    bool createApplication()
    {
        Application::setAppDir(QStringLiteral(GRAPHIA_BUILD_DIR));
        _application = std::make_unique<Application>(nullptr, false);

        return _dir.isValid();
    }

    void closeDocument()
    {
        _document.reset();
        _graphDisplay.reset();
    }

    void destroyApplication()
    {
        closeDocument();
        _application.reset();
    }

    QUrl write(const QString& fileName, const QByteArray& content)
    {
        auto filePath = _dir.filePath(fileName);
        QFile file(filePath);
        if(!file.open(QIODevice::WriteOnly) || file.write(content) != content.size())
            return {};

        return QUrl::fromLocalFile(filePath);
    }

    bool open(const QUrl& url, const QString& type, const QString& pluginName,
        const QVariantMap& parameters = {})
    {
        // Destroy any previous Document first, as the UI does when its tab is closed
        _document.reset();

        _graphDisplay = std::make_unique<GraphDisplay>();
        _document = std::make_unique<Document>();
        _document->setProperty("application", QVariant::fromValue(_application.get()));
        _document->setProperty("graphDisplay", QVariant::fromValue(_graphDisplay.get()));

        QSignalSpy loadComplete(_document.get(), &Document::loadComplete);
        if(!_document->openUrl(url, type, pluginName, parameters))
            return false;

        if(!QTest::qWaitFor([&] { return !loadComplete.isEmpty(); }, Timeout))
            return false;

        if(!loadComplete.at(0).at(1).toBool())
            return false;

        // As the renderer does once it has been created, which is otherwise never, as
        // the display isn't in a window; until then, the Document is always busy. The
        // Document only learns of this once events are processed, and pauses the layout
        // when commands start only if it has, so wait for it, as there is in the UI
        QSignalSpy busyChanged(_document.get(), &Document::busyChanged);
        QMetaObject::invokeMethod(_graphDisplay.get(), "onRendererInitialised");

        return QTest::qWaitFor([&] { return !busyChanged.isEmpty() && !_document->busy(); }, Timeout);
    }

    bool waitForCommands()
    {
        return QTest::qWaitFor([this] { return !_document->busy(); }, Timeout);
    }

    // Does something that executes commands, then waits for them to finish
    template<typename Fn>
    bool execute(const Fn& fn)
    {
        QSignalSpy commandsFinished(_document.get(), &Document::commandsFinished);
        fn();

        return QTest::qWaitFor([&] { return !commandsFinished.isEmpty(); }, Timeout) && waitForCommands();
    }

    GraphModel* graphModel() { return dynamic_cast<GraphModel*>(_document->graphModel()); }
    const Graph& graph() { return graphModel()->graph(); }

    NodeId nodeId(const QString& name)
    {
        for(auto nodeId : graphModel()->mutableGraph().nodeIds())
        {
            if(graphModel()->nodeName(nodeId) == name)
                return nodeId;
        }

        return {};
    }

    // The names of the selected nodes, sorted
    QStringList selected()
    {
        const auto nodeIds = _document->selectionManager()->selectedNodes();

        QStringList names;
        names.reserve(static_cast<qsizetype>(nodeIds.size()));
        for(auto nodeId : nodeIds)
            names.append(graphModel()->nodeName(nodeId));

        names.sort();
        return names;
    }

    // The names of the nodes in the transformed graph, sorted
    QStringList nodeNames()
    {
        QStringList names;
        names.reserve(static_cast<qsizetype>(graph().nodeIds().size()));
        for(auto nodeId : graph().nodeIds())
            names.append(graphModel()->nodeName(nodeId));

        names.sort();
        return names;
    }
};

#endif // DOCUMENTTEST_H
