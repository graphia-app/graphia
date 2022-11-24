/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#ifndef BASEPLUGIN_H
#define BASEPLUGIN_H

#include "shared/plugins/iplugin.h"
#include "shared/graph/igraph.h"
#include "shared/graph/igraphmodel.h"
#include "shared/ui/idocument.h"
#include "shared/ui/iselectionmanager.h"
#include "shared/commands/icommandmanager.h"
#include "shared/loading/iparserthread.h"
#include "shared/loading/urltypes.h"

#include <memory>

#include <QObject>

// The plugins never see these types; they only need to know they exist
// for the purposes of signal connection
class Graph;
class SelectionManager;

// Provide some default implementations that most plugins will require, to avoid
// them having to repetitively implement the same functionality themselves
class BasePluginInstance : public QObject, public IPluginInstance
{
    Q_OBJECT

private:
    const IPlugin* _plugin = nullptr;
    IDocument* _document = nullptr;
    IGraphModel* _graphModel = nullptr;
    ISelectionManager* _selectionManager = nullptr;
    ICommandManager* _commandManager = nullptr;

public:
    void initialise(const IPlugin* plugin, IDocument* document,
                    const IParserThread* parserThread) override
    {
        _plugin = plugin;
        _document = document;
        _graphModel = document->graphModel();
        _selectionManager = document->selectionManager();
        _commandManager = document->commandManager();

        const auto* graphQObject = dynamic_cast<const QObject*>(&_graphModel->graph());
        Q_ASSERT(graphQObject != nullptr);

        connect(graphQObject, SIGNAL(graphWillChange(const Graph*)),
                this, SIGNAL(graphWillChange()), Qt::DirectConnection);

        connect(graphQObject, SIGNAL(nodeAdded(const Graph*,NodeId)),
                this, SLOT(onNodeAdded(const Graph*,NodeId)), Qt::DirectConnection);
        connect(graphQObject, SIGNAL(nodeRemoved(const Graph*,NodeId)),
                this, SLOT(onNodeRemoved(const Graph*,NodeId)), Qt::DirectConnection);
        connect(graphQObject, SIGNAL(edgeAdded(const Graph*,EdgeId)),
                this, SLOT(onEdgeAdded(const Graph*,EdgeId)), Qt::DirectConnection);
        connect(graphQObject, SIGNAL(edgeRemoved(const Graph*,EdgeId)),
                this, SLOT(onEdgeRemoved(const Graph*,EdgeId)), Qt::DirectConnection);

        connect(graphQObject, SIGNAL(graphChanged(const Graph*,bool)),
                this, SIGNAL(graphChanged()), Qt::DirectConnection);

        const auto* selectionManagerQObject = dynamic_cast<const QObject*>(_selectionManager);
        Q_ASSERT(selectionManagerQObject != nullptr);

        connect(selectionManagerQObject, SIGNAL(selectionChanged(const SelectionManager*)),
                this, SLOT(onSelectionChanged(const SelectionManager*)), Qt::DirectConnection);

        const auto* graphModelQObject = dynamic_cast<const QObject*>(_graphModel);
        Q_ASSERT(graphModelQObject != nullptr);

        connect(graphModelQObject, SIGNAL(visualsChanged(VisualChangeFlags,VisualChangeFlags)),
                this, SLOT(onVisualsChanged(VisualChangeFlags,VisualChangeFlags)), Qt::DirectConnection);

        const auto* parserThreadQObject = dynamic_cast<const QObject*>(parserThread);
        Q_ASSERT(parserThreadQObject != nullptr);

        connect(parserThreadQObject, SIGNAL(success(IParser*)),
                this, SLOT(onLoadSuccess()), Qt::DirectConnection);
    }

    // Ignore all settings, by default
    void applyParameter(const QString&, const QVariant&) override {}

    QStringList defaultTransforms() const override { return {}; }
    QStringList defaultVisualisations() const override { return {}; }

    // Save and restore no state, by default
    QByteArray save(IMutableGraph&, Progressable&) const override { return {}; }
    bool load(const QByteArray&, int, IMutableGraph&, IParser&) override { return true; }

    void setSaveRequired() { emit saveRequired(); }

    const IPlugin* plugin() override { return _plugin; }
    IDocument* document() { return _document; }
    const IDocument* document() const { return _document; }
    IGraphModel* graphModel() { return _graphModel; }
    const IGraphModel* graphModel() const { return _graphModel; }
    ISelectionManager* selectionManager() { return _selectionManager; }
    const ISelectionManager* selectionManager() const { return _selectionManager; }
    ICommandManager* commandManager() { return _commandManager; }

private slots:
    void onNodeAdded(const Graph*, NodeId nodeId)       { emit nodeAdded(nodeId); }
    void onNodeRemoved(const Graph*, NodeId nodeId)     { emit nodeRemoved(nodeId); }
    void onEdgeAdded(const Graph*, EdgeId edgeId)       { emit edgeAdded(edgeId); }
    void onEdgeRemoved(const Graph*, EdgeId edgeId)     { emit edgeRemoved(edgeId); }

    void onSelectionChanged(const SelectionManager*)    { emit selectionChanged(_selectionManager); }
    void onVisualsChanged(VisualChangeFlags nodeChange,
        VisualChangeFlags edgeChange)                   { emit visualsChanged(nodeChange, edgeChange); }

    void onLoadSuccess()                                { emit loadSuccess(); }

signals:
    void graphWillChange();

    void nodeAdded(NodeId);
    void nodeRemoved(NodeId);
    void edgeAdded(EdgeId);
    void edgeRemoved(EdgeId);

    void graphChanged();

    void selectionChanged(const ISelectionManager* selectionManager);
    void visualsChanged(VisualChangeFlags nodeChange, VisualChangeFlags edgeChange);

    void loadSuccess();

    void saveRequired();
};

// Plugins can inherit from this to avoid having to reimplement the same createInstance member function
// over and over again, in the case where the instance class constructor takes no parameters
template<typename T>
class PluginInstanceProvider : public virtual IPluginInstanceProvider
{
public:
    ~PluginInstanceProvider() override = default;

    std::unique_ptr<IPluginInstance> createInstance() override { return std::make_unique<T>(); }
};

class BasePlugin : public QObject, public IPlugin, public UrlTypes
{
    Q_OBJECT
    Q_INTERFACES(IPlugin)

private:
    const IApplication* _application = nullptr;

public:
    void initialise(const IApplication* application) override
    {
        _application = application;
    }

    // Default empty image
    QString imageSource() const override { return {}; }

    // Default to no settings UI
    QString parametersQmlPath(const QString&) const override { return {}; }

    // Default to no UI
    QString qmlPath() const override { return {}; }

    // Default to directed graphs
    bool directed() const override { return true; }

    const IApplication* application() const override { return _application; }

    QObject* ptr() override { return this; }
};

#endif // BASEPLUGIN_H
