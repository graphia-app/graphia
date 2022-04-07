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

#include "document.h"

#include "application.h"
#include "preferences.h"

#include "attributes/enrichmentcalculator.h"

#include "shared/plugins/iplugin.h"
#include "shared/utils/flags.h"
#include "shared/utils/color.h"
#include "shared/utils/string.h"
#include "shared/loading/userelementdata.h"
#include "shared/utils/static_block.h"

#include "graph/mutablegraph.h"
#include "graph/graphmodel.h"

#include "loading/parserthread.h"
#include "loading/nativeloader.h"
#include "loading/nativesaver.h"
#include "loading/isaver.h"

#include "layout/forcedirectedlayout.h"
#include "layout/layout.h"
#include "layout/collision.h"

#include "commands/applytransformscommand.h"
#include "commands/applyvisualisationscommand.h"
#include "commands/deletenodescommand.h"
#include "commands/cloneattributecommand.h"
#include "commands/editattributecommand.h"
#include "commands/removeattributescommand.h"
#include "commands/importattributescommand.h"
#include "commands/selectnodescommand.h"

#include "transform/graphtransform.h"
#include "transform/graphtransformconfigparser.h"
#include "ui/visualisations/visualisationinfo.h"
#include "ui/visualisations/visualisationconfigparser.h"

#include "attributes/conditionfncreator.h"
#include "attributes/attributeedits.h"

#include "searchmanager.h"
#include "selectionmanager.h"
#include "graphquickitem.h"

#include "../crashhandler.h"

#include <json_helper.h>
#include <numeric>

#include <QQmlProperty>
#include <QMetaObject>
#include <QFile>
#include <QAbstractItemModel>
#include <QMessageBox>
#include <QCollator>
#include <QApplication>
#include <QElapsedTimer>
#include <QVariantList>
#include <QVector>
#include <QClipboard>
#include <QQmlEngine>

QColor Document::contrastingColorForBackground()
{
    auto backColor = u::pref(QStringLiteral("visuals/backgroundColor")).value<QColor>();
    return u::contrastingColor(backColor);
}

Document::Document(QObject* parent) :
    QObject(parent),
    _graphChanging(false),
    _layoutRequired(true)
{}

Document::~Document()
{
    if(_graphFileParserThread != nullptr)
    {
        // Stop any load that's in progress
        _graphFileParserThread->cancel();
        _graphFileParserThread->wait();
    }

    // Wait for any executing commands to complete
    _commandManager.wait();

    // Execute anything pending (primarily to avoid deadlock)
    executeDeferred();

    // This must be called from the main thread before deletion
    if(_gpuComputeThread != nullptr)
        _gpuComputeThread->destroySurface();
}

const IGraphModel* Document::graphModel() const
{
    return _graphModel.get();
}

IGraphModel* Document::graphModel()
{
    return _graphModel.get();
}

const ISelectionManager* Document::selectionManager() const
{
    return _selectionManager.get();
}

ISelectionManager* Document::selectionManager()
{
    return _selectionManager.get();
}

MessageBoxButton Document::messageBox(MessageBoxIcon icon, const QString& title,
    const QString& text, Flags<MessageBoxButton> buttons)
{
    MessageBoxButton result;

    executeOnMainThreadAndWait([&]
    {
        QApplication::alert(nullptr);
        QMessageBox messageBox(static_cast<QMessageBox::Icon>(icon), title, text,
            static_cast<QMessageBox::StandardButton>(*buttons));

        messageBox.exec();
        result = static_cast<MessageBoxButton>(messageBox.result());
    });

    return result;
}

bool Document::commandInProgress() const
{
    return !_loadComplete || _commandManager.busy();
}

bool Document::busy() const
{
    if(!_graphQuickItem->initialised())
        return true;

    return commandInProgress() || graphChanging() ||
        _graphQuickItem->updating() || _graphQuickItem->transitioning() ||
        _graphQuickItem->interacting();
}

bool Document::editable() const
{
    if(_graphModel == nullptr)
        return false;

    return _graphModel->editable();
}

bool Document::directed() const
{
    if(_graphModel == nullptr)
        return true;

    return _graphModel->directed();
}

bool Document::graphChanging() const
{
    return _graphChanging;
}

void Document::maybeEmitBusyChanged()
{
    if(qEnvironmentVariableIntValue("BUSY_STATE_DEBUG") != 0)
    {
        static QElapsedTimer timer;

        if(busy())
        {
            if(!_previousBusy && (!timer.isValid() || timer.elapsed() > 250))
                qDebug() << "----";

            timer.restart();
        }

        qDebug().noquote() << QStringLiteral("busy %1%2%3%4%5%6").arg(
            (commandInProgress() ?                  "C" : "."),
            (graphChanging() ?                      "G" : "."),
            (_graphQuickItem->updating() ?          "U" : "."),
            (_graphQuickItem->interacting() ?       "I" : "."),
            (_graphQuickItem->transitioning() ?     "T" : "."),
            (busy() != _previousBusy ? (busy() ?    " +" : " -") : "  "));
    }

    if(busy() != _previousBusy)
    {
        _previousBusy = busy();
        emit busyChanged();
    }
}

int Document::commandProgress() const
{
    if(!_loadComplete)
        return _loadProgress;

    return _commandManager.commandProgress();
}

QString Document::commandVerb() const
{
    if(_graphModel == nullptr)
        return {};

    auto phase = _graphModel->graph().phase();

    if(!_loadComplete)
    {
        if(!phase.isEmpty())
            return QString(tr("Loading %1 (%2)").arg(_title, phase));

        return QString(tr("Loading %1").arg(_title));
    }

    if(!phase.isEmpty())
        return QString(tr("%1 (%2)")).arg(_commandManager.commandVerb(), phase);

    return _commandManager.commandVerb();
}

bool Document::commandIsCancellable() const
{
    return !_loadComplete || _commandManager.commandIsCancellable();
}

bool Document::commandIsCancelling() const
{
    if(!_loadComplete && _graphFileParserThread != nullptr && _graphFileParserThread->cancelled())
        return true;

    return _commandManager.commandIsCancelling();
}

QString Document::layoutName() const
{
    if(_layoutThread != nullptr)
        return _layoutThread->layoutName();

    return {};
}

QString Document::layoutDisplayName() const
{
    if(_layoutThread != nullptr)
        return _layoutThread->layoutDisplayName();

    return {};
}

std::vector<LayoutSetting>& Document::layoutSettings() const
{
    return _layoutThread->settings();
}

void Document::updateLayoutDimensionality()
{
    auto newDimensionality = _graphQuickItem->projection() == Projection::TwoDee ?
        Layout::Dimensionality::TwoDee :
        Layout::Dimensionality::ThreeDee;

    if(newDimensionality != _layoutThread->dimensionalityMode())
    {
        _layoutThread->setDimensionalityMode(newDimensionality);
        _layoutRequired = true;
    }
}

void Document::updateLayoutState()
{
    if(!busy() && !_userLayoutPaused && _layoutRequired)
    {
        _layoutThread->resume();
        _layoutRequired = false;
    }
    else
    {
        if(!_userLayoutPaused && !_layoutThread->paused())
            _layoutRequired = true;

        _layoutThread->pauseAndWait();
    }
}

LayoutPauseState Document::layoutPauseState()
{
    if(_layoutThread == nullptr)
        return LayoutPauseState::Paused;

    if(_userLayoutPaused)
        return LayoutPauseState::Paused;

    if(_layoutThread->finished())
        return LayoutPauseState::RunningFinished;

    return LayoutPauseState::Running;
}

void Document::setUserLayoutPaused(bool userLayoutPaused)
{
    if(busy() || _userLayoutPaused == userLayoutPaused)
        return;

    _userLayoutPaused = userLayoutPaused;
    _layoutRequired = true;
    emit layoutPauseStateChanged();

    updateLayoutState();

    setSaveRequired();
}

void Document::resumeLayout()
{
    setUserLayoutPaused(false);
}

void Document::toggleLayout()
{
    setUserLayoutPaused(!_userLayoutPaused);
}

bool Document::canUndo() const
{
    return !busy() && _commandManager.canUndo();
}

QString Document::nextUndoAction() const
{
    return _commandManager.nextUndoAction();
}

bool Document::canRedo() const
{
    return !busy() && _commandManager.canRedo();
}

QString Document::nextRedoAction() const
{
    return _commandManager.nextRedoAction();
}

bool Document::canResetView() const
{
    return !busy() && !_graphQuickItem->viewIsReset();
}

bool Document::canEnterOverviewMode() const
{
    return !busy() && _graphQuickItem->canEnterOverviewMode();
}

void Document::setTitle(const QString& title)
{
    if(title != _title)
    {
        _title = title;
        emit titleChanged();
    }
}

void Document::setStatus(const QString& status)
{
    if(status != _status)
    {
        _status = status;
        emit statusChanged();
    }
}

void Document::setTransforms(const QStringList& transforms)
{
    _graphTransforms = transforms;

    _graphTransformsFromUI = transforms;
    emit transformsChanged();

    setSaveRequired();
}

void Document::setVisualisations(const QStringList& visualisations)
{
    _visualisations = visualisations;
    _graphModel->buildVisualisations(_visualisations);

    _visualisationsFromUI = visualisations;
    emit visualisationsChanged();

    setSaveRequired();
}


float Document::fps() const
{
    if(_graphQuickItem != nullptr)
        return _graphQuickItem->fps();

    return 0.0f;
}

QObject* Document::pluginInstance()
{
    // This will return nullptr if _pluginInstance is not a QObject, which is
    // allowed, but means that the UI can't interact with it
    return dynamic_cast<QObject*>(_pluginInstance.get());
}

QString Document::pluginQmlPath() const
{
    return _graphModel != nullptr ? _graphModel->pluginQmlPath() : QString();
}

QStringList Document::bookmarks() const
{
    QStringList list;
    list.reserve(static_cast<int>(_bookmarks.size()));

    for(const auto& name : u::keysFor(_bookmarks))
        list.append(name);

    QCollator sorter;
    sorter.setNumericMode(true);
    sorter.setCaseSensitivity(Qt::CaseInsensitive);

    std::sort(list.begin(), list.end(),
    [&](const auto& a, const auto& b)
    {
        return sorter.compare(a, b) < 0;
    });

    return list;
}

NodeIdSet Document::nodeIdsForBookmark(const QString& name) const
{
    if(u::containsKey(_bookmarks, name))
        return _bookmarks.at(name);

    return {};
}

static bool transformIsPinned(const QString& transform)
{
    GraphTransformConfigParser p;

    if(!p.parse(transform)) return false;
    return p.result().isFlagSet(QStringLiteral("pinned"));
}

static QStringList sortedTransforms(QStringList transforms)
{
    // Sort so that the pinned transforms go last
    std::stable_sort(transforms.begin(), transforms.end(),
    [](const QString& a, const QString& b)
    {
        bool aPinned = transformIsPinned(a);
        bool bPinned = transformIsPinned(b);

        if(aPinned && !bPinned)
            return false;

        if(!aPinned && bPinned)
            return true; // NOLINT

        return false;
    });

    return transforms;
}

QStringList Document::graphTransformConfigurationsFromUI() const
{
    return sortedTransforms(_graphTransformsFromUI);
}

bool Document::hasValidEdgeTextVisualisation() const
{
    return _graphModel != nullptr ? _graphModel->hasValidEdgeTextVisualisation() : false;
}

void Document::initialiseLayoutSettingsModel()
{
    const auto& settings = _layoutThread->settings();

    QStringList layoutSettingNames;
    layoutSettingNames.reserve(static_cast<int>(settings.size()));

    for(const auto& setting : settings)
        layoutSettingNames.append(setting.name());

    _layoutSettingNames = layoutSettingNames;
    emit layoutSettingNamesChanged();
}

bool Document::openUrl(const QUrl& url, const QString& type, QString pluginName, const QVariantMap& parameters)
{
    std::unique_ptr<IParser> parser;
    Loader* loader = nullptr;

    if(type == Application::NativeFileType)
    {
        parser = std::make_unique<Loader>();
        loader = dynamic_cast<Loader*>(parser.get());
        pluginName = Loader::pluginNameFor(url);
    }

    auto* plugin = _application->pluginForName(pluginName);

    if(plugin == nullptr)
        return false;

    if(type != Application::NativeFileType)
    {
        setLog(tr("Loaded from %1, as type %2, using plugin %3 (version %4)")
            .arg(url.toString(), type, pluginName)
            .arg(plugin->dataVersion()));
    }

    _pluginName = pluginName;
    emit pluginNameChanged();

    setTitle(url.fileName());
    emit commandInProgressChanged();
    emit busyChanged();
    emit commandVerbChanged(); // Show Loading message

    _graphModel = std::make_unique<GraphModel>(url.fileName(), plugin);

    _gpuComputeThread = std::make_unique<GPUComputeThread>();
    _graphFileParserThread = std::make_unique<ParserThread>(*_graphModel, url);

    _selectionManager = std::make_unique<SelectionManager>(*_graphModel);
    _searchManager = std::make_unique<SearchManager>(*_graphModel);

    _pluginInstance = plugin->createInstance();

    const auto& keys = parameters.keys();
    for(const auto& name : keys)
        _pluginInstance->applyParameter(name, parameters.value(name));

    // Connect this before the plugin is initialised, in case it needs to see
    // all the available attributes during initialisation
    connect(_graphFileParserThread.get(), &ParserThread::success, [this]
    {
        _graphModel->userNodeData().exposeAsAttributes(*_graphModel);
        _graphModel->userEdgeData().exposeAsAttributes(*_graphModel);
    });

    _pluginInstance->initialise(plugin, this, _graphFileParserThread.get());

    // The plugin won't necessarily have the saveRequired signal or in fact be
    // a QObject at all, hence this convoluted and defensive runtime connection
    const auto* pluginInstanceQObject = dynamic_cast<const QObject*>(_pluginInstance.get());
    if(pluginInstanceQObject != nullptr)
    {
        auto signature = QMetaObject::normalizedSignature("saveRequired()");
        bool hasSignal = pluginInstanceQObject->metaObject()->indexOfSignal(signature) >= 0;

        if(hasSignal)
        {
            connect(pluginInstanceQObject, SIGNAL(saveRequired()),
                    this, SLOT(onPluginSaveRequired()), Qt::DirectConnection);
        }
    }

    connect(&_preferencesWatcher, &PreferencesWatcher::preferenceChanged,
        this, &Document::onPreferenceChanged, Qt::DirectConnection);

    connect(&_graphModel->graph(), &Graph::graphChanged, [this] { _searchManager->refresh(); });

    connect(this, &Document::taskAddedToExecutor, this, &Document::executeDeferred);

    connect(&_graphModel->mutableGraph(), &Graph::phaseChanged, this, &Document::commandVerbChanged);

    connect(_graphModel.get(), &GraphModel::attributesChanged, this, &Document::attributesChanged); // clazy:exclude=connect-non-signal
    connect(_graphModel.get(), &GraphModel::attributesChanged, this, &Document::setSaveRequired); // clazy:exclude=connect-non-signal

    emit pluginInstanceChanged();

    if(parser == nullptr)
    {
        // If we don't yet have a parser, we need to ask the plugin for one
        parser = _pluginInstance->parserForUrlTypeName(type);

        if(parser == nullptr)
        {
            qDebug() << "Plugin does not provide parser";
            return false;
        }
    }

    // Build the transforms and visualisations in the parser thread since they may
    // take time to compute and we may as well roll them into the loading process
    if(loader != nullptr)
    {
        loader->setPluginInstance(_pluginInstance.get());

        connect(_graphFileParserThread.get(), &ParserThread::success,
        [this](IParser* completedParser)
        {
            auto* completedLoader = dynamic_cast<Loader*>(completedParser);

            Q_ASSERT(completedLoader != nullptr);
            if(completedLoader == nullptr)
                return;

            _graphTransforms = _graphModel->transformsWithMissingParametersSetToDefault(
                completedLoader->transforms());

            _visualisations = completedLoader->visualisations();
            _bookmarks = completedLoader->bookmarks();
            setLog(completedLoader->log());

            _graphModel->buildTransforms(_graphTransforms);

            if(completedParser->cancelled())
                return;

            _graphModel->buildVisualisations(_visualisations);

            //FIXME make use of this when we can switch algorithms = completedLoader->layoutName();
            _loadedLayoutSettings = completedLoader->layoutSettings();

            const auto* nodePositions = completedLoader->nodePositions();
            if(nodePositions != nullptr)
                _startingNodePositions = std::make_unique<ExactNodePositions>(*nodePositions);

            _userLayoutPaused = completedLoader->layoutPaused();

            setProjection(static_cast<int>(completedLoader->projection()));
            setShading3D(static_cast<int>(completedLoader->shading()));

            _uiData = completedLoader->uiData();

            _pluginUiData = completedLoader->pluginUiData();
            _pluginUiDataVersion = completedLoader->pluginUiDataVersion();

            const auto& enrichmentTableData = completedLoader->enrichmentTableData();
            executeOnMainThread([this, enrichmentTableData]()
            {
                for(const auto& data : enrichmentTableData)
                {
                    auto* tableModel = new EnrichmentTableModel(this);
                    tableModel->setTableData(data._table, data._selectionA, data._selectionB);
                    _enrichmentTableModels.append(QVariant::fromValue(tableModel));
                }

                emit enrichmentTableModelsChanged();
            });
        });
    }
    else
    {
        connect(_graphFileParserThread.get(), &ParserThread::success,
        [this](IParser* completedParser)
        {
            auto parserLog = completedParser->log();

            if(!parserLog.isEmpty())
                setLog(log() + "\n\n" + parserLog);

            const auto& graph = _graphModel->mutableGraph();
            setLog(log() + QStringLiteral("\n\nNodes: %1 Edges: %2")
                .arg(graph.numNodes()).arg(graph.numEdges()));

            _graphTransforms = _graphModel->transformsWithMissingParametersSetToDefault(
                sortedTransforms(_pluginInstance->defaultTransforms()));
            _visualisations = _pluginInstance->defaultVisualisations();

            _graphModel->buildTransforms(_graphTransforms);

            for(auto& visualisation : _visualisations)
            {
                VisualisationConfigParser p;
                bool success = p.parse(visualisation);
                Q_ASSERT(success);

                const auto& attributeName = p.result()._attributeName;
                auto valueType = _graphModel->attributeValueByName(attributeName).valueType();
                const auto& channelName = p.result()._channelName;

                auto defaultParameters = _graphModel->visualisationDefaultParameters(valueType, channelName);

                if(!defaultParameters.isEmpty())
                {
                    visualisation += QStringLiteral(" with");

                    const auto& parameterKeys = defaultParameters.keys();
                    for(const auto& key : parameterKeys)
                    {
                        auto value = u::escapeQuotes(defaultParameters.value(key).toString());
                        visualisation += QStringLiteral(R"( %1 = "%2")").arg(key, value);
                    }
                }
            }

            _graphModel->buildVisualisations(_visualisations);
        });
    }

    connect(_graphFileParserThread.get(), &ParserThread::progress, this, &Document::onLoadProgress);
    connect(_graphFileParserThread.get(), &ParserThread::complete, this, &Document::onLoadComplete);
    connect(_graphFileParserThread.get(), &ParserThread::complete, this, &Document::loadComplete);
    connect(_graphFileParserThread.get(), &ParserThread::cancelledChanged,
            this, &Document::commandIsCancellingChanged);
    _graphFileParserThread->start(std::move(parser));

    return true;
}

void Document::saveFile(const QUrl& fileUrl, const QString& saverName, const QByteArray& uiData,
                        const QByteArray& pluginUiData)
{
    auto* factory = _application->saverFactoryByName(saverName);
    if(factory != nullptr)
    {
        _commandManager.executeOnce(
        [=, this](Command& command) mutable
        {
            auto saver = factory->create(fileUrl, this, _pluginInstance.get(), uiData, pluginUiData);
            saver->setProgressFn([&command](int percentage){ command.setProgress(percentage); });
            bool success = saver->save();
            emit saveComplete(success, fileUrl, saverName);
            return success;
        },
        tr("Saving %1").arg(fileUrl.fileName()));

        _saveRequired = false;
        emit saveRequiredChanged();
    }
    else
    {
        QMessageBox::critical(nullptr, tr("Save Error"), QStringLiteral("%1 %2")
            .arg(tr("Unable to find registered saver with name:"), saverName));
    }
}

void Document::onPreferenceChanged(const QString& key, const QVariant&)
{
    if(key == QStringLiteral("visuals/backgroundColor"))
        emit contrastingColorChanged();
    else if(key == QStringLiteral("visuals/showEdgeText"))
    {
        // showEdgeText affects the warning state of TextVisualisationChannel
        setVisualisations(_visualisations);
    }
}

void Document::onLoadProgress(int percentage)
{
    _loadProgress = percentage;
    emit commandProgressChanged();
    emit commandVerbChanged();
}

void Document::onLoadComplete(const QUrl&, bool success)
{
    _graphFileParserThread->reset();

    if(!success)
    {
        setFailureReason(_graphFileParserThread->failureReason());
        emit failureReasonChanged();

        // Give up now because the whole Document object will be
        // destroyed soon anyway
        return;
    }

    // Final tasks before load is considered complete
    _graphModel->initialiseAttributeRanges();
    _graphModel->initialiseSharedAttributeValues();

    setTransforms(_graphTransforms);
    setVisualisations(_visualisations);

    if(!_bookmarks.empty())
        emit bookmarksChanged();

    _layoutThread = std::make_unique<LayoutThread>(*_graphModel, std::make_unique<ForceDirectedLayoutFactory>(_graphModel.get()));

    for(const auto& layoutSetting : _loadedLayoutSettings)
        _layoutThread->setSettingValue(layoutSetting._name, layoutSetting._value);

    updateLayoutDimensionality();

    if(_startingNodePositions != nullptr)
    {
        _layoutThread->setStartingNodePositions(*_startingNodePositions);
        _startingNodePositions.reset();
    }

    _loadComplete = true;
    emit commandInProgressChanged();
    emit commandIsCancellingChanged();
    emit commandIsCancellableChanged();
    emit busyChanged();
    emit editableChanged();
    emit directedChanged();
    emit commandVerbChanged(); // Stop showing loading message
    emit nodeSizeChanged();
    emit edgeSizeChanged();

    // Load UI saved data
    if(_uiData.size() > 0)
        emit uiDataChanged(_uiData);

    // This causes the plugin UI to be loaded
    emit pluginQmlPathChanged(_pluginUiData, _pluginUiDataVersion);

    connect(_layoutThread.get(), &LayoutThread::pausedChanged, this, &Document::layoutPauseStateChanged);
    connect(_layoutThread.get(), &LayoutThread::settingChanged, [this] { _layoutRequired = true; });
    connect(_layoutThread.get(), &LayoutThread::settingChanged, this, &Document::updateLayoutState);
    connect(_layoutThread.get(), &LayoutThread::settingChanged, this, &Document::layoutSettingChanged);
    _layoutThread->addAllComponents();
    initialiseLayoutSettingsModel();

    // Force a layout in the unlikely event that nothing else does
    _layoutRequired = true;
    updateLayoutState();

    emit layoutNameChanged();
    emit layoutDisplayNameChanged();

    _graphQuickItem->initialise(_graphModel.get(), &_commandManager, _selectionManager.get(), _gpuComputeThread.get());

    connect(_graphQuickItem, &GraphQuickItem::initialisedChanged, this, &Document::maybeEmitBusyChanged, Qt::QueuedConnection);
    connect(_graphQuickItem, &GraphQuickItem::updatingChanged, this, &Document::maybeEmitBusyChanged, Qt::QueuedConnection);
    connect(_graphQuickItem, &GraphQuickItem::interactingChanged, this, &Document::maybeEmitBusyChanged, Qt::QueuedConnection);
    connect(_graphQuickItem, &GraphQuickItem::transitioningChanged, this, &Document::maybeEmitBusyChanged, Qt::QueuedConnection);
    connect(_graphQuickItem, &GraphQuickItem::viewIsResetChanged, this, &Document::canResetViewChanged);
    connect(_graphQuickItem, &GraphQuickItem::canEnterOverviewModeChanged, this, &Document::canEnterOverviewModeChanged);
    connect(_graphQuickItem, &GraphQuickItem::fpsChanged, this, &Document::fpsChanged);
    connect(_graphQuickItem, &GraphQuickItem::visibleComponentIndexChanged, this, &Document::numInvisibleNodesSelectedChanged);

    connect(&_commandManager, &CommandManager::started, this, &Document::maybeEmitBusyChanged, Qt::DirectConnection);
    connect(&_commandManager, &CommandManager::started, this, &Document::commandInProgressChanged);

    connect(&_commandManager, &CommandManager::started, _graphQuickItem, &GraphQuickItem::commandsStarted);
    connect(&_commandManager, &CommandManager::finished, _graphQuickItem, &GraphQuickItem::commandsFinished);

    connect(&_commandManager, &CommandManager::finished, this, &Document::commandInProgressChanged);
    connect(&_commandManager, &CommandManager::finished, this, &Document::maybeEmitBusyChanged, Qt::DirectConnection);

    connect(this, &Document::busyChanged, [this] { if(!busy()) updateLayoutDimensionality(); });

    connect(this, &Document::busyChanged, this, &Document::updateLayoutState, Qt::DirectConnection);

    connect(this, &Document::busyChanged, this, &Document::editableChanged);
    connect(this, &Document::busyChanged, this, &Document::canUndoChanged);
    connect(this, &Document::busyChanged, this, &Document::canRedoChanged);
    connect(this, &Document::busyChanged, this, &Document::canEnterOverviewModeChanged);
    connect(this, &Document::busyChanged, this, &Document::canResetViewChanged);

    connect(this, &Document::busyChanged, this, &Document::onBusyChanged);

    connect(&_commandManager, &CommandManager::commandProgressChanged, this, &Document::commandProgressChanged);
    connect(&_commandManager, &CommandManager::commandVerbChanged, this, &Document::commandVerbChanged);
    connect(&_commandManager, &CommandManager::commandIsCancellableChanged, this, &Document::commandIsCancellableChanged);
    connect(&_commandManager, &CommandManager::commandIsCancellingChanged, this, &Document::commandIsCancellingChanged);

    connect(&_commandManager, &CommandManager::finished, this, &Document::commandsFinished);
    connect(&_commandManager, &CommandManager::finished, this, &Document::canUndoChanged);
    connect(&_commandManager, &CommandManager::finished, this, &Document::nextUndoActionChanged);
    connect(&_commandManager, &CommandManager::finished, this, &Document::canRedoChanged);
    connect(&_commandManager, &CommandManager::finished, this, &Document::nextRedoActionChanged);
    connect(&_commandManager, &CommandManager::commandCompleted,
    [this](bool, const QString&, const QString& pastParticiple)
    {
        // Commands might set the phase and neglect to unset it
        _graphModel->mutableGraph().clearPhase();

        setStatus(pastParticiple);
    });

    connect(&_commandManager, &CommandManager::commandStackCleared, this, &Document::canUndoChanged);
    connect(&_commandManager, &CommandManager::commandStackCleared, this, &Document::nextUndoActionChanged);
    connect(&_commandManager, &CommandManager::commandStackCleared, this, &Document::canRedoChanged);
    connect(&_commandManager, &CommandManager::commandStackCleared, this, &Document::nextRedoActionChanged);

    connect(_selectionManager.get(), &SelectionManager::selectionChanged, this, &Document::onSelectionChanged);
    connect(_selectionManager.get(), &SelectionManager::selectionChanged,
            _graphModel.get(), &GraphModel::onSelectionChanged);
    connect(_selectionManager.get(), &SelectionManager::selectionChanged, this, &Document::numNodesSelectedChanged);
    connect(_selectionManager.get(), &SelectionManager::selectionChanged, this, &Document::numHeadNodesSelectedChanged);
    connect(_selectionManager.get(), &SelectionManager::selectionChanged, this, &Document::numInvisibleNodesSelectedChanged);
    connect(_selectionManager.get(), &SelectionManager::selectionChanged, this, &Document::selectedNodeIdsChanged);
    connect(_selectionManager.get(), &SelectionManager::selectionChanged, this, &Document::selectedHeadNodeIdsChanged);
    connect(_selectionManager.get(), &SelectionManager::nodesMaskChanged, this, &Document::nodesMaskActiveChanged);

    connect(_searchManager.get(), &SearchManager::foundNodeIdsChanged, this, &Document::onFoundNodeIdsChanged);
    connect(_searchManager.get(), &SearchManager::foundNodeIdsChanged,
            _graphModel.get(), &GraphModel::onFoundNodeIdsChanged);

    connect(_layoutThread.get(), &LayoutThread::executed, _graphQuickItem, &GraphQuickItem::onLayoutChanged);

    connect(_graphModel.get(), &GraphModel::visualsChanged, this, &Document::hasValidEdgeTextVisualisationChanged); // clazy:exclude=connect-non-signal
    connect(_graphModel.get(), &GraphModel::rebuildRequired, // clazy:exclude=connect-non-signal
    [this](bool transforms, bool visualisations)
    {
        ICommandPtrsVector commands;

        if(transforms)
        {
            commands.emplace_back(
                std::make_unique<ApplyTransformsCommand>(
                _graphModel.get(), _selectionManager.get(), this,
                _graphTransforms, graphTransformConfigurationsFromUI()));
        }

        if(visualisations)
        {
            commands.emplace_back(
                std::make_unique<ApplyVisualisationsCommand>(
                _graphModel.get(), this,
                _visualisations, _visualisationsFromUI));
        }

        if(!commands.empty())
        {
            _commandManager.execute(ExecutePolicy::OnceMutate, std::move(commands),
                {
                    tr("Apply Transforms and Visualisations"),
                    tr("Applying Transforms and Visualisations")
                });
        }
    });

    connect(&_graphModel->graph(), &Graph::graphWillChange, this, &Document::graphWillChange);
    connect(&_graphModel->graph(), &Graph::graphChanged, this, &Document::graphChanged);

    connect(&_graphModel->graph(), &Graph::graphWillChange, [this]
    {
        bool graphChangingWillChange = !_graphChanging;
        _graphChanging = true;

        if(graphChangingWillChange)
            emit graphChangingChanged();

        maybeEmitBusyChanged();
    });

    connect(&_graphModel->graph(), &Graph::graphChanged, [this]
    (const Graph*, bool changeOccurred)
    {
        bool graphChangingWillChange = _graphChanging;
        _graphChanging = false;

        if(graphChangingWillChange)
            emit graphChangingChanged();

        _layoutRequired = changeOccurred || _layoutRequired;
        maybeEmitBusyChanged();

        // If the graph has changed outside of a Command, then our new state is
        // inconsistent wrt the CommandManager, so throw away our undo history
        if(!commandInProgress())
            _commandManager.clearCommandStack();
    });

    connect(&_graphModel->graph(), &Graph::graphChanged, [this]
    {
        executeOnMainThreadAndWait([this]
        {
            // If the graph changes then so do our visualisations
            setVisualisations(_visualisations);
        }, QStringLiteral("Document graphChanged"));

        setSaveRequired();
    });

    connect(&_graphModel->graph(), &Graph::graphChanged, &_commandManager,
            &CommandManager::onGraphChanged, Qt::DirectConnection);

    connect(&_graphModel->mutableGraph(), &Graph::graphChanged,
    [this]
    {
        executeOnMainThreadAndWait([this]
        {
            // This is only called in order to force the UI to refresh the transform
            // controls, in case the attribute ranges have changed
            setTransforms(_graphTransforms);
        }, QStringLiteral("Document (mutable) graphChanged"));

        setSaveRequired();
    });

    connect(this, &Document::enrichmentTableModelsChanged, this, &Document::setSaveRequired);

    _graphModel->enableVisualUpdates();

    setStatus(QString(tr("Loaded %1 (%2 nodes, %3 edges, %4 components)"))
        .arg(_graphModel->name())
        .arg(_graphModel->graph().numNodes())
        .arg(_graphModel->graph().numEdges())
        .arg(_graphModel->graph().numComponents()));
}

void Document::onBusyChanged() const
{
    if(!busy())
        QApplication::alert(nullptr);
}

bool Document::nodeIsSelected(QmlNodeId nodeId) const
{
    if(_selectionManager == nullptr)
        return false;

    return _selectionManager->nodeIsSelected(nodeId);
}

void Document::selectAll()
{
    if(busy() || _selectionManager == nullptr)
        return;

    _commandManager.executeOnce(
    [this](Command& command)
    {
        bool nodesSelected = _selectionManager->selectAllNodes();
        command.setPastParticiple(_selectionManager->numNodesSelectedAsString());
        return nodesSelected;
    }, tr("Selecting All"));
}

void Document::selectAllFound()
{
    if(busy() || _selectionManager == nullptr)
        return;

    _selectionManager->setNodesMask(_searchManager->foundNodeIds(), true);
    selectAll();
}

void Document::selectAllVisible()
{
    if(busy() || _selectionManager == nullptr)
        return;

    if(canEnterOverviewMode())
    {
        auto componentId = _graphQuickItem->focusedComponentId();
        const auto* component = _graphModel->graph().componentById(componentId);
        const auto& nodeIds = component->nodeIds();

        _commandManager.execute(ExecutePolicy::Once,
            makeSelectNodesCommand(_selectionManager.get(), nodeIds));
    }
    else
        selectAll();
}

void Document::selectNone()
{
    if(busy() || _selectionManager == nullptr)
        return;

    if(!_selectionManager->selectedNodes().empty())
    {
        _commandManager.executeOnce(
            [this](Command&) { return _selectionManager->clearNodeSelection(); },
            tr("Selecting None"));
    }
}

void Document::selectSources()
{
    if(busy() || _selectionManager == nullptr)
        return;

    auto selectedNodeIds = _selectionManager->selectedNodes();
    Q_ASSERT(!selectedNodeIds.empty());
    NodeIdSet nodeIds = selectedNodeIds;

    for(auto nodeId : selectedNodeIds)
    {
        auto sources = _graphModel->graph().sourcesOf(nodeId);
        nodeIds.insert(sources.begin(), sources.end());
    }

    _commandManager.execute(ExecutePolicy::Once,
        makeSelectNodesCommand(_selectionManager.get(),
        nodeIds, SelectNodesClear::SelectionAndMask));
}

// NOLINTNEXTLINE readability-make-member-function-const
void Document::selectSourcesOf(QmlNodeId nodeId)
{
    if(busy())
        return;

    NodeIdSet nodeIds = {nodeId};

    auto sources = _graphModel->graph().sourcesOf(nodeId);
    nodeIds.insert(sources.begin(), sources.end());

    _commandManager.execute(ExecutePolicy::Once,
        makeSelectNodesCommand(_selectionManager.get(),
        nodeIds, SelectNodesClear::SelectionAndMask));
}

void Document::selectTargets()
{
    if(busy() || _selectionManager == nullptr)
        return;

    auto selectedNodeIds = _selectionManager->selectedNodes();
    Q_ASSERT(!selectedNodeIds.empty());
    NodeIdSet nodeIds = selectedNodeIds;

    for(auto nodeId : selectedNodeIds)
    {
        auto targets = _graphModel->graph().targetsOf(nodeId);
        nodeIds.insert(targets.begin(), targets.end());
    }

    _commandManager.execute(ExecutePolicy::Once,
        makeSelectNodesCommand(_selectionManager.get(),
        nodeIds, SelectNodesClear::SelectionAndMask));
}

// NOLINTNEXTLINE readability-make-member-function-const
void Document::selectTargetsOf(QmlNodeId nodeId)
{
    if(busy())
        return;

    NodeIdSet nodeIds = {nodeId};

    auto targets = _graphModel->graph().targetsOf(nodeId);
    nodeIds.insert(targets.begin(), targets.end());

    _commandManager.execute(ExecutePolicy::Once,
        makeSelectNodesCommand(_selectionManager.get(),
        nodeIds, SelectNodesClear::SelectionAndMask));
}

void Document::selectNeighbours()
{
    if(busy() || _selectionManager == nullptr)
        return;

    auto selectedNodeIds = _selectionManager->selectedNodes();
    Q_ASSERT(!selectedNodeIds.empty());
    NodeIdSet nodeIds = selectedNodeIds;

    for(auto nodeId : selectedNodeIds)
    {
        auto neighbours = _graphModel->graph().neighboursOf(nodeId);
        nodeIds.insert(neighbours.begin(), neighbours.end());
    }

    _commandManager.execute(ExecutePolicy::Once,
        makeSelectNodesCommand(_selectionManager.get(),
        nodeIds, SelectNodesClear::SelectionAndMask));
}

// NOLINTNEXTLINE readability-make-member-function-const
void Document::selectNeighboursOf(QmlNodeId nodeId)
{
    if(busy())
        return;

    NodeIdSet nodeIds = {nodeId};

    auto neighbours = _graphModel->graph().neighboursOf(nodeId);
    nodeIds.insert(neighbours.begin(), neighbours.end());

    _commandManager.execute(ExecutePolicy::Once,
        makeSelectNodesCommand(_selectionManager.get(),
        nodeIds, SelectNodesClear::SelectionAndMask));
}

void Document::selectBySharedAttributeValue(const QString& attributeName, QmlNodeId qmlNodeId)
{
    if(busy() || _selectionManager == nullptr)
        return;

    NodeIdSet nodeIdSet;

    if(!qmlNodeId.isNull())
        nodeIdSet.emplace(qmlNodeId);
    else
        nodeIdSet = _selectionManager->selectedNodes();

    if(nodeIdSet.empty())
        return;

    const auto* attribute = _graphModel->attributeByName(attributeName);
    Q_ASSERT(attribute != nullptr);

    std::set<QString> attributeValues;

    for(auto nodeId : nodeIdSet)
        attributeValues.emplace(attribute->stringValueOf(nodeId));

    QString term;
    for(const auto& attributeValue : attributeValues)
    {
        if(!term.isEmpty())
            term.append('|');

        term.append(QRegularExpression::escape(attributeValue));
    }

    term = QStringLiteral("^(%1)$").arg(term);

    auto conditionFn = CreateConditionFnFor::node(*attribute,
        ConditionFnOp::String::MatchesRegex, term);

    std::vector<NodeId> nodeIds;

    for(auto nodeId : _graphModel->graph().nodeIds())
    {
        if(conditionFn(nodeId))
            nodeIds.emplace_back(nodeId);
    }

    if(!nodeIds.empty())
    {
        _commandManager.execute(ExecutePolicy::Once,
            makeSelectNodesCommand(_selectionManager.get(),
            nodeIds, SelectNodesClear::SelectionAndMask));
    }
}

void Document::invertSelection()
{
    if(busy() || _selectionManager == nullptr)
        return;

    _commandManager.executeOnce(
    [this](Command& command)
    {
        _selectionManager->clearNodesMask();
        _selectionManager->invertNodeSelection();
        command.setPastParticiple(_selectionManager->numNodesSelectedAsString());
    }, tr("Inverting Selection"));
}

// NOLINTNEXTLINE readability-make-member-function-const
void Document::undo()
{
    if(busy())
        return;

    _commandManager.undo();
}

// NOLINTNEXTLINE readability-make-member-function-const
void Document::redo()
{
    if(busy())
        return;

    _commandManager.redo();
}

// NOLINTNEXTLINE readability-make-member-function-const
void Document::rollback()
{
    if(busy())
        return;

    _commandManager.rollback();
}

// NOLINTNEXTLINE readability-make-member-function-const
void Document::deleteNode(QmlNodeId nodeId)
{
    if(busy())
        return;

    _commandManager.execute(ExecutePolicy::Add,
        std::make_unique<DeleteNodesCommand>(_graphModel.get(),
        _selectionManager.get(), NodeIdSet{nodeId}));
}

// NOLINTNEXTLINE readability-make-member-function-const
void Document::deleteSelectedNodes()
{
    if(busy())
        return;

    if(_selectionManager->selectedNodes().empty())
        return;

    _commandManager.execute(ExecutePolicy::Add,
        std::make_unique<DeleteNodesCommand>(_graphModel.get(),
        _selectionManager.get(), _selectionManager->selectedNodes()));
}

void Document::resetView()
{
    if(busy())
        return;

    _graphQuickItem->resetView();
}

void Document::switchToOverviewMode(bool doTransition)
{
    if(busy())
        return;

    _graphQuickItem->switchToOverviewMode(doTransition);
}

int Document::projection() const
{
    return static_cast<int>(_graphQuickItem->projection());
}

void Document::setProjection(int _projection)
{
    _graphQuickItem->setProjection(static_cast<Projection>(_projection));
}

int Document::shading2D() const
{
    return static_cast<int>(_graphQuickItem->shading2D());
}

void Document::setShading2D(int _shading2D)
{
    _graphQuickItem->setShading2D(static_cast<Shading>(_shading2D));
}

int Document::shading3D() const
{
    return static_cast<int>(_graphQuickItem->shading3D());
}

void Document::setShading3D(int _shading3D)
{
    _graphQuickItem->setShading3D(static_cast<Shading>(_shading3D));
}

void Document::gotoPrevComponent()
{
    const auto& componentIds = _graphModel->graph().componentIds();
    auto focusedComponentId = _graphQuickItem->focusedComponentId();

    if(busy() || componentIds.empty())
        return;

    if(!focusedComponentId.isNull())
    {
        auto it = std::find(componentIds.begin(), componentIds.end(), focusedComponentId);

        if(it != componentIds.begin())
            --it;
        else
            it = std::prev(componentIds.end());

        _graphQuickItem->moveFocusToComponent(*it);
    }
    else
        _graphQuickItem->moveFocusToComponent(componentIds.back());
}

void Document::gotoNextComponent()
{
    const auto& componentIds = _graphModel->graph().componentIds();
    auto focusedComponentId = _graphQuickItem->focusedComponentId();

    if(busy() || componentIds.empty())
        return;

    if(!focusedComponentId.isNull())
    {
        auto it = std::find(componentIds.begin(), componentIds.end(), focusedComponentId);

        if(std::next(it) != componentIds.end())
            ++it;
        else
            it = componentIds.begin();

        _graphQuickItem->moveFocusToComponent(*it);
    }
    else
        _graphQuickItem->moveFocusToComponent(componentIds.front());
}

void Document::find(const QString& term, int options, const QStringList& attributeNames, int findSelectStyle)
{
    if(_searchManager == nullptr)
        return;

    _commandManager.executeOnce([=, this](Command&)
    {
        _searchManager->findNodes(term, static_cast<FindOptions>(options),
            attributeNames, static_cast<FindSelectStyle>(findSelectStyle));
    });
}

void Document::resetFind()
{
    if(_searchManager == nullptr)
        return;

    find({}, {}, {}, {});
}

static bool shouldMoveFindFocus(bool inOverviewMode)
{
    return u::pref(QStringLiteral("misc/focusFoundNodes")).toBool() &&
        (!inOverviewMode || u::pref(QStringLiteral("misc/focusFoundComponents")).toBool());
}

void Document::selectAndFocusNode(NodeId nodeId)
{
    _commandManager.execute(ExecutePolicy::Once,
        makeSelectNodeCommand(_selectionManager.get(), nodeId),
        [=, this](Command&)
        {
            executeOnMainThread([=, this]
            {
                if(shouldMoveFindFocus(_graphQuickItem->inOverviewMode()))
                    _graphQuickItem->moveFocusToNode(nodeId);
            });
        });
}

void Document::selectAndFocusNodes(const std::vector<NodeId>& nodeIds)
{
    _commandManager.execute(ExecutePolicy::Once,
        makeSelectNodesCommand(_selectionManager.get(), nodeIds),
        [=, this](Command&)
        {
            executeOnMainThread([=, this]
            {
                if(shouldMoveFindFocus(_graphQuickItem->inOverviewMode()))
                    _graphQuickItem->moveFocusToNodes(nodeIds);
            });
        });
}

void Document::selectAndFocusNodes(const NodeIdSet& nodeIds)
{
    selectAndFocusNodes(u::vectorFrom(nodeIds));
}

void Document::moveFocusToNode(NodeId nodeId)
{
    _graphQuickItem->moveFocusToNode(nodeId);
}

void Document::moveFocusToNodes(const std::vector<NodeId>& nodeIds)
{
    _graphQuickItem->moveFocusToNodes(nodeIds);
}

void Document::clearHighlightedNodes()
{
    if(_graphModel != nullptr)
        _graphModel->clearHighlightedNodes();
}

void Document::highlightNodes(const NodeIdSet& nodeIds)
{
    if(_graphModel != nullptr)
        _graphModel->highlightNodes(nodeIds);
}

void Document::reportProblem(const QString& description) const
{
    S(CrashHandler)->submitMinidump(description);
}

const QString& Document::log() const
{
    return _log;
}

void Document::setLog(const QString& log)
{
    if(log != _log)
    {
        _log = log;
        emit logChanged();
    }
}

// NOLINTNEXTLINE readability-make-member-function-const
void Document::setSaveRequired()
{
    if(!_loadComplete)
        return;

    _saveRequired = true;
    emit saveRequiredChanged();
}

int Document::numNodesSelected() const
{
    if(_selectionManager != nullptr)
        return _selectionManager->numNodesSelected();

    return 0;
}

int Document::numHeadNodesSelected() const
{
    int numNodes = 0;

    if(_selectionManager != nullptr)
    {
        for(auto nodeId : _selectionManager->selectedNodes())
        {
            if(_graphModel->graph().typeOf(nodeId) != MultiElementType::Tail)
                numNodes++;
        }
    }

    return numNodes;
}

int Document::numInvisibleNodesSelected() const
{
    if(_selectionManager != nullptr)
    {
        auto selectedNodes = _selectionManager->selectedNodes();

        for(auto it = selectedNodes.begin(); it != selectedNodes.end(); /*NO OP*/)
        {
            auto componentIdOfNode = _graphModel->graph().componentIdOfNode(*it);
            if(_graphQuickItem->focusedComponentId() == componentIdOfNode)
                it = selectedNodes.erase(it);
            else
                ++it;
        }

        return static_cast<int>(selectedNodes.size());
    }

    return 0;
}

QVariantList Document::selectedNodeIds() const
{
    QVariantList nodes;

    if(_selectionManager != nullptr)
    {
        const auto& selectedNodes = _selectionManager->selectedNodes();
        nodes.reserve(static_cast<int>(selectedNodes.size()));

        for(auto nodeId : selectedNodes)
            nodes.append(QVariant::fromValue<QmlNodeId>(nodeId));
    }

    return nodes;
}

QVariantList Document::selectedHeadNodeIds() const
{
    QVariantList nodes;

    if(_selectionManager != nullptr)
    {
        for(auto nodeId : _selectionManager->selectedNodes())
        {
            if(_graphModel->graph().typeOf(nodeId) != MultiElementType::Tail)
                nodes.append(QVariant::fromValue<QmlNodeId>(nodeId));
        }
    }

    return nodes;
}

void Document::selectFirstFound()
{
    selectAndFocusNode(*_foundNodeIds.begin());
}

void Document::selectNextFound()
{
    _selectionManager->setNodesMask(_searchManager->foundNodeIds());
    selectAndFocusNode(incrementFoundIt());
}

void Document::selectPrevFound()
{
    _selectionManager->setNodesMask(_searchManager->foundNodeIds());
    selectAndFocusNode(decrementFoundIt());
}

void Document::updateFoundIndex(bool reselectIfInvalidated)
{
    // For the purposes of updating the found index, we only care
    // about the heads of merged node sets, so find them
    std::vector<NodeId> selectedHeadNodes;
    for(auto selectedNodeId : _selectionManager->selectedNodes())
    {
        if(_graphModel->graph().typeOf(selectedNodeId) != MultiElementType::Tail)
            selectedHeadNodes.emplace_back(selectedNodeId);
    }

    if(selectedHeadNodes.size() == 1)
    {
        auto nodeId = *selectedHeadNodes.begin();
        auto foundIt = std::find(_foundNodeIds.begin(), _foundNodeIds.end(), nodeId);

        if(reselectIfInvalidated && foundIt == _foundNodeIds.end())
        {
            // If the previous found NodeId /was/ in our found list, but isn't anymore,
            // grab a new one
            selectFirstFound();
        }
        else if(foundIt != _foundNodeIds.end())
        {
            // If the selected NodeId is still in the found NodeIds, then
            // adjust the index appropriately
            setFoundIt(foundIt);
        }
        else
        {
            _foundItValid = false;
            emit foundIndexChanged();
        }
    }
    else
    {
        // The iterator is still valid, so don't invalidate
        // it as we might want to come back to it later
        emit foundIndexChanged();
    }

    if(_numNodesFoundChanged)
    {
        _numNodesFoundChanged = false;
        emit numNodesFoundChanged();
    }
}

void Document::selectByAttributeValue(const QString& attributeName, const QString& term)
{
    std::vector<NodeId> nodeIds;

    auto parsedAttributeName = Attribute::parseAttributeName(attributeName);
    if(u::contains(_graphModel->availableAttributeNames(), parsedAttributeName._name))
    {
        const auto& attribute = _graphModel->attributeValueByName(parsedAttributeName._name);

        auto conditionFn = CreateConditionFnFor::node(attribute, ConditionFnOp::String::MatchesRegex, term);
        if(conditionFn != nullptr)
        {
            for(auto nodeId : _graphModel->graph().nodeIds())
            {
                if(_graphModel->graph().typeOf(nodeId) == MultiElementType::Tail)
                    continue;

                if(conditionFn(nodeId))
                    nodeIds.emplace_back(nodeId);
            }
        }
    }

    if(nodeIds.empty())
    {
        _selectionManager->clearNodeSelection();
        return;
    }

    selectAndFocusNodes(nodeIds);
}

QString Document::nodeName(QmlNodeId nodeId) const
{
    if(_graphModel == nullptr || nodeId.isNull())
        return {};

    return _graphModel->nodeName(nodeId);
}

void Document::onSelectionChanged(const SelectionManager*)
{
    updateFoundIndex(false);
}

void Document::onFoundNodeIdsChanged(const SearchManager* searchManager)
{
    _numNodesFoundChanged = _foundNodeIds.size() != searchManager->foundNodeIds().size();
    _foundNodeIds.clear();

    if(searchManager->foundNodeIds().empty())
    {
        bool nodesWereFound = _foundItValid || searchManager->selectStyle() == FindSelectStyle::All;
        if(nodesWereFound && searchManager->active())
            _selectionManager->clearNodeSelection();

        _selectionManager->clearNodesMask();

        _foundItValid = false;
        emit foundIndexChanged();
        emit numNodesFoundChanged();

        return;
    }

    _selectionManager->setNodesMask(searchManager->foundNodeIds(), false);
    _foundNodeIds = u::vectorFrom(searchManager->foundNodeIds());

    std::sort(_foundNodeIds.begin(), _foundNodeIds.end(), [this](auto a, auto b)
    {
        auto componentIdA = _graphModel->graph().componentIdOfNode(a);
        auto componentIdB = _graphModel->graph().componentIdOfNode(b);

        if(componentIdA == componentIdB)
            return a < b;

        return componentIdA < componentIdB;
    });

    // _foundNodeIds is potentially in a different memory location,
    // so the iterator is now invalid
    _foundItValid = false;

    if(_searchManager->selectStyle() == FindSelectStyle::All)
        selectAndFocusNodes(u::vectorFrom(_searchManager->foundNodeIds()));
    else if(_selectionManager->selectedNodes().empty())
        selectFirstFound();
    else
        updateFoundIndex(true);
}

void Document::onPluginSaveRequired()
{
    setSaveRequired();
}

void Document::executeDeferred()
{
    _deferredExecutor.execute();
}

int Document::foundIndex() const
{
    if(!_foundNodeIds.empty() && _foundItValid && numHeadNodesSelected() == 1)
        return static_cast<int>(std::distance(_foundNodeIds.begin(), _foundIt));

    return -1;
}

int Document::numNodesFound() const
{
    if(_searchManager != nullptr)
        return static_cast<int>(_searchManager->foundNodeIds().size());

    return 0;
}

bool Document::nodesMaskActive() const
{
    if(_selectionManager != nullptr)
        return _selectionManager->nodesMaskActive();

    return false;
}

// NOLINTNEXTLINE readability-make-member-function-const
void Document::setFoundIt(std::vector<NodeId>::const_iterator foundIt)
{
    bool changed = !_foundItValid || (_foundIt != foundIt);
    _foundIt = foundIt;

    bool oldFoundItValid = _foundItValid;
    _foundItValid = (_foundIt != _foundNodeIds.end());

    if(!changed)
        changed = (_foundItValid != oldFoundItValid);

    if(changed)
        emit foundIndexChanged();
}

// NOLINTNEXTLINE readability-make-member-function-const
NodeId Document::incrementFoundIt()
{
    auto foundIt = _foundIt;
    auto foundItValid = _foundItValid;

    do
    {
        if(foundItValid && std::next(foundIt) != _foundNodeIds.end())
            ++foundIt;
        else
        {
            foundIt = _foundNodeIds.begin();
            foundItValid = true;
        }
    }
    while(_graphModel->graph().typeOf(*foundIt) == MultiElementType::Tail);

    return *foundIt;
}

// NOLINTNEXTLINE readability-make-member-function-const
NodeId Document::decrementFoundIt()
{
    auto foundIt = _foundIt;
    auto foundItValid = _foundItValid;

    do
    {
        if(foundItValid && foundIt != _foundNodeIds.begin())
            --foundIt;
        else
        {
            foundIt = std::prev(_foundNodeIds.end());
            foundItValid = true;
        }
    }
    while(_graphModel->graph().typeOf(*foundIt) == MultiElementType::Tail);

    return *foundIt;
}

size_t Document::executeOnMainThread(DeferredExecutor::TaskFn task,
    const QString& description)
{
    auto numTasksInQueue = _deferredExecutor.enqueue(std::move(task), description);
    emit taskAddedToExecutor();
    return numTasksInQueue;
}

void Document::executeOnMainThreadAndWait(DeferredExecutor::TaskFn task,
    const QString& description)
{
    auto numTasksInQueue = executeOnMainThread(std::move(task), description);
    _deferredExecutor.waitFor(numTasksInQueue);
}

AvailableTransformsModel* Document::availableTransforms() const
{
    if(_graphModel != nullptr)
    {
        // The caller takes ownership and is responsible for deleting the model
        return new AvailableTransformsModel(*_graphModel, nullptr);
    }

    return nullptr;
}


QStringList Document::availableAttributeNames(int _elementTypes, int _valueTypes,
    int _skipFlags, const QStringList& skipAttributeNames) const
{
    if(_graphModel == nullptr)
        return {};

    auto elementTypeFlags = Flags<ElementType>(static_cast<ElementType>(_elementTypes));
    auto valueTypes = static_cast<ValueType>(_valueTypes);
    auto skipFlags = static_cast<AttributeFlag>(_skipFlags);

    auto attributeNames = _graphModel->availableAttributeNames(
        *elementTypeFlags, valueTypes, skipFlags, skipAttributeNames);

    if(elementTypeFlags.test(ElementType::Edge))
    {
        auto sourceAttributes =_graphModel->availableAttributeNames(ElementType::Node, valueTypes);
        auto targetAttributes = sourceAttributes;
        sourceAttributes.replaceInStrings(QRegularExpression(QStringLiteral("^")), QStringLiteral("source."));
        targetAttributes.replaceInStrings(QRegularExpression(QStringLiteral("^")), QStringLiteral("target."));
        attributeNames += sourceAttributes + targetAttributes;
    }

    return attributeNames;
}

AvailableAttributesModel* Document::availableAttributesModel(int elementTypes, int valueTypes,
    int skipFlags, const QStringList& skipAttributeNames) const
{
    if(_graphModel != nullptr)
    {
        // The caller takes ownership and is responsible for deleting the model
        return new AvailableAttributesModel(*_graphModel, nullptr,
            static_cast<ElementType>(elementTypes), static_cast<ValueType>(valueTypes),
            static_cast<AttributeFlag>(skipFlags), skipAttributeNames);
    }

    return nullptr;
}

bool Document::attributeExists(const QString& attributeName) const
{
    if(_graphModel != nullptr)
        return _graphModel->attributeExists(attributeName);

    return false;
}

QVariantMap Document::transform(const QString& transformName) const
{
    QVariantMap map;

    if(_graphModel != nullptr)
    {
        const auto* transformFactory = _graphModel->transformFactory(transformName);

        if(transformFactory == nullptr)
            return map;

        auto elementType = transformFactory->elementType();

        map.insert(QStringLiteral("elementType"), static_cast<int>(elementType));
        map.insert(QStringLiteral("name"), transformName);
        map.insert(QStringLiteral("description"), transformFactory->description());
        map.insert(QStringLiteral("image"), transformFactory->image());
        map.insert(QStringLiteral("requiresCondition"), transformFactory->requiresCondition());

        QStringList attributeParameterNames;
        QVariantList attributeParameters;
        for(const auto& attributeParameter : transformFactory->attributeParameters())
        {
            QVariantMap attributeParameterMap = transformAttributeParameter(transformName, attributeParameter.name());
            attributeParameterNames.append(attributeParameter.name());
            attributeParameters.append(attributeParameterMap);
        }
        map.insert(QStringLiteral("attributeParameterNames"), attributeParameterNames);
        map.insert(QStringLiteral("attributeParameters"), attributeParameters);

        QStringList parameterNames;
        QVariantMap parameters;
        for(const auto& parameter : transformFactory->parameters())
        {
            QVariantMap parameterMap = transformParameter(transformName, parameter.name());
            parameterNames.append(parameter.name());
            parameters.insert(parameter.name(), parameterMap);
        }
        map.insert(QStringLiteral("parameterNames"), parameterNames);
        map.insert(QStringLiteral("parameters"), parameters);

        QVariantMap defaultVisualisations;
        for(const auto& defaultVisualisation : transformFactory->defaultVisualisations())
        {
            QVariantMap defaultVisualisationMap;
            auto attributeName = Attribute::enquoteAttributeName(defaultVisualisation._attributeName);
            defaultVisualisationMap.insert(QStringLiteral("name"), attributeName);
            defaultVisualisationMap.insert(QStringLiteral("flags"), static_cast<int>(*defaultVisualisation._attributeFlags));
            defaultVisualisationMap.insert(QStringLiteral("valueType"), static_cast<int>(defaultVisualisation._attributeValueType));
            defaultVisualisationMap.insert(QStringLiteral("channelName"), defaultVisualisation._channel);
            defaultVisualisations.insert(attributeName, defaultVisualisationMap);
        }
        map.insert(QStringLiteral("defaultVisualisations"), defaultVisualisations);
    }

    return map;
}

bool Document::hasTransformInfo() const
{
    return _graphModel != nullptr ? _graphModel->hasTransformInfo() : false;
}

QVariantMap Document::transformInfoAtIndex(int index) const
{
    QVariantMap map;

    map.insert(QStringLiteral("alertType"), static_cast<int>(AlertType::None));
    map.insert(QStringLiteral("alertText"), "");

    if(_graphModel == nullptr)
        return map;

    const auto& transformInfo = _graphModel->transformInfoAtIndex(index);

    auto alerts = transformInfo.alerts();

    if(alerts.empty())
        return map;

    std::sort(alerts.begin(), alerts.end(),
    [](auto& a, auto& b)
    {
        return a._type > b._type;
    });

    auto& transformAlert = alerts.at(0);

    map.insert(QStringLiteral("alertType"), static_cast<int>(transformAlert._type));
    map.insert(QStringLiteral("alertText"), transformAlert._text);

    return map;
}

bool Document::opIsUnary(const QString& op) const
{
    return _graphModel != nullptr ? _graphModel->opIsUnary(op) : false;
}

QVariantMap Document::transformParameter(const QString& transformName, const QString& parameterName) const
{
    QVariantMap map;

    if(_graphModel == nullptr)
        return map;

    const auto* transformFactory = _graphModel->transformFactory(transformName);

    if(transformFactory == nullptr)
        return map;

    auto parameter = transformFactory->parameter(parameterName);
    if(!parameter.name().isEmpty())
    {
        map.insert(QStringLiteral("name"), parameter.name());
        map.insert(QStringLiteral("valueType"), static_cast<int>(parameter.type()));

        map.insert(QStringLiteral("hasRange"), parameter.hasRange());
        map.insert(QStringLiteral("hasMinimumValue"), parameter.hasMin());
        map.insert(QStringLiteral("hasMaximumValue"), parameter.hasMax());

        if(parameter.hasMin()) map.insert(QStringLiteral("minimumValue"), parameter.min());
        if(parameter.hasMax()) map.insert(QStringLiteral("maximumValue"), parameter.max());

        map.insert(QStringLiteral("description"), parameter.description());
        map.insert(QStringLiteral("initialValue"), parameter.initialValue());

        if(!parameter.validatorRegex().isEmpty())
            map.insert(QStringLiteral("validatorRegex"), parameter.validatorRegex());

        // If it's a StringList parameter, select the first one by default
        if(parameter.type() == ValueType::StringList)
            map.insert(QStringLiteral("initialIndex"), 0);
    }

    return map;
}

QVariantMap Document::transformAttributeParameter(const QString& transformName, const QString& parameterName) const
{
    QVariantMap map;

    if(_graphModel == nullptr)
        return map;

    const auto* transformFactory = _graphModel->transformFactory(transformName);

    if(transformFactory == nullptr)
        return map;

    auto parameter = transformFactory->attributeParameter(parameterName);
    if(!parameter.name().isEmpty())
    {
        map.insert(QStringLiteral("name"), parameter.name());
        map.insert(QStringLiteral("elementType"), static_cast<int>(parameter.elementType()));
        map.insert(QStringLiteral("valueType"), static_cast<int>(parameter.valueType()));
        map.insert(QStringLiteral("description"), parameter.description());
    }

    return map;
}

QVariantMap Document::attribute(const QString& attributeName) const
{
    QVariantMap map;

    if(_graphModel == nullptr)
        return map;

    auto parsedAttributeName = Attribute::parseAttributeName(attributeName);
    if(u::contains(_graphModel->availableAttributeNames(), parsedAttributeName._name))
    {
        auto attribute = _graphModel->attributeValueByName(attributeName);

        const char* prefix = "";
        switch(parsedAttributeName._type)
        {
        case Attribute::EdgeNodeType::Source: prefix = "source."; break;
        case Attribute::EdgeNodeType::Target: prefix = "target."; break;
        default: break;
        }

        map.insert(QStringLiteral("name"), QStringLiteral("%1%2")
            .arg(prefix, parsedAttributeName._name));

        bool hasParameter = attribute.hasParameter();
        map.insert(QStringLiteral("hasParameter"), hasParameter);

        if(hasParameter)
        {
            if(!parsedAttributeName._parameter.isEmpty())
                GraphModel::calculateAttributeRange(&graphModel()->mutableGraph(), attribute);

            map.insert(QStringLiteral("parameterValue"), parsedAttributeName._parameter);
            map.insert(QStringLiteral("isValid"), !parsedAttributeName._parameter.isEmpty());
            map.insert(QStringLiteral("validParameterValues"), attribute.validParameterValues());
        }
        else
            map.insert(QStringLiteral("isValid"), true);

        map.insert(QStringLiteral("flags"), static_cast<int>(attribute.flags()));
        map.insert(QStringLiteral("valueType"), static_cast<int>(attribute.valueType()));
        map.insert(QStringLiteral("elementType"), static_cast<int>(attribute.elementType()));
        map.insert(QStringLiteral("userDefined"), attribute.userDefined());
        map.insert(QStringLiteral("editable"), attribute.editable());

        map.insert(QStringLiteral("hasRange"), attribute.numericRange().hasRange());
        map.insert(QStringLiteral("hasMinimumValue"), attribute.numericRange().hasMin());
        map.insert(QStringLiteral("hasMaximumValue"), attribute.numericRange().hasMax());

        if(attribute.numericRange().hasMin()) map.insert(QStringLiteral("minimumValue"), attribute.numericRange().min());
        if(attribute.numericRange().hasMax()) map.insert(QStringLiteral("maximumValue"), attribute.numericRange().max());

        map.insert(QStringLiteral("description"), attribute.description());
        map.insert(QStringLiteral("ops"), _graphModel->avaliableConditionFnOps(parsedAttributeName._name));

        QStringList sharedValues;
        const auto& sharedValueCounts = attribute.sharedValues();
        sharedValues.reserve(static_cast<int>(sharedValueCounts.size()));
        for(const auto& sharedValueCount : sharedValueCounts)
            sharedValues.append(sharedValueCount._value);

        map.insert(QStringLiteral("sharedValues"), sharedValues);
    }
    else
        map.insert(QStringLiteral("isValid"), false);

    return map;
}

AvailableAttributesModel* Document::attributesSimilarTo(const QString& attributeName, int skipFlags) const
{
    if(attributeName.isEmpty())
        return nullptr;

    if(_graphModel == nullptr)
        return nullptr;

    auto parsedAttributeName = Attribute::parseAttributeName(attributeName);
    if(u::contains(_graphModel->availableAttributeNames(), parsedAttributeName._name))
    {
        const auto& attribute = _graphModel->attributeValueByName(attributeName);
        const auto& underlyingAttribute = _graphModel->attributeValueByName(parsedAttributeName._name);

        auto valueTypeFlags = Flags<ValueType>(underlyingAttribute.valueType());

        // For similarity purposes, treat Int and Float as the same
        if(valueTypeFlags.anyOf(ValueType::Int, ValueType::Float))
            valueTypeFlags.set(ValueType::Numerical);

        return availableAttributesModel(static_cast<int>(attribute.elementType()),
            static_cast<int>(*valueTypeFlags), skipFlags);
    }

    return nullptr;
}

QStringList Document::allAttributeValues(const QString& attributeName) const
{
    if(attributeName.isEmpty())
        return {};

    if(_graphModel == nullptr)
        return {};

    const auto* attribute = _graphModel->attributeByName(attributeName);
    if(attribute == nullptr)
        return {};

    QStringList attributeValues;

    if(attribute->elementType() == ElementType::Node)
    {
        attributeValues.reserve(_graphModel->mutableGraph().numNodes());

        for(auto nodeId : _graphModel->mutableGraph().nodeIds())
            attributeValues.append(attribute->stringValueOf(nodeId));
    }
    else if(attribute->elementType() == ElementType::Edge)
    {
        attributeValues.reserve(_graphModel->mutableGraph().numEdges());

        for(auto edgeId : _graphModel->mutableGraph().edgeIds())
            attributeValues.append(attribute->stringValueOf(edgeId));
    }

    return attributeValues;
}

QStringList Document::createdAttributeNamesAtTransformIndexOrLater(int firstIndex) const
{
    return u::toQStringList(_graphModel->createdAttributeNamesAtTransformIndexOrLater(firstIndex));
}

// NOLINTNEXTLINE readability-convert-member-functions-to-static
QVariantMap Document::parseGraphTransform(const QString& transform) const
{
    GraphTransformConfigParser p;
    if(p.parse(transform))
        return p.result().asVariantMap();

    return {};
}

bool Document::graphTransformIsValid(const QString& transform) const
{
    return _graphModel != nullptr ? _graphModel->graphTransformIsValid(transform) : false;
}

void Document::setGraphTransform(int index, const QString& transform)
{
    Q_ASSERT(index >= 0 && index < _graphTransformsFromUI.count());
    _graphTransformsFromUI[index] = transform;
    emit transformsChanged();
}

void Document::removeGraphTransform(int index)
{
    Q_ASSERT(index >= 0 && index < _graphTransformsFromUI.count());
    _graphTransformsFromUI.removeAt(index);
    emit transformsChanged();
}

// This tests two transform lists to determine if replacing one with the
// other would actually result in a different transformation
static bool transformsDiffer(const QStringList& a, const QStringList& b)
{
    if(a.length() != b.length())
        return true;

    GraphTransformConfigParser p;

    for(int i = 0; i < a.length(); i++)
    {
        GraphTransformConfig ai, bi;

        if(p.parse(a[i]))
            ai = p.result();

        if(p.parse(b[i]))
            bi = p.result();

        if(ai != bi)
            return true;
    }

    return false;
}

void Document::moveGraphTransform(int from, int to)
{
    if(_graphModel == nullptr)
        return;

    if(from < 0 || from >= _graphTransforms.size())
        return;

    if(to < 0 || to >= _graphTransforms.size())
        return;

    QStringList newGraphTransforms = _graphTransforms;
    newGraphTransforms.move(from, to);

    _commandManager.execute(ExecutePolicy::Add,
        std::make_unique<ApplyTransformsCommand>(
        _graphModel.get(), _selectionManager.get(), this,
        _graphTransforms, newGraphTransforms));
}

QStringList Document::availableVisualisationChannelNames(int valueType) const
{
    return _graphModel != nullptr ? _graphModel->availableVisualisationChannelNames(
                                        static_cast<ValueType>(valueType)) : QStringList();
}

bool Document::visualisationChannelAllowsMapping(const QString& channelName) const
{
    return _graphModel != nullptr ? _graphModel->visualisationChannelAllowsMapping(channelName) : false;
}

QStringList Document::visualisationDescription(const QString& attributeName, const QStringList& channelNames) const
{
    return _graphModel != nullptr ? _graphModel->visualisationDescription(attributeName, channelNames) : QStringList();
}

bool Document::hasVisualisationInfo() const
{
    return _graphModel != nullptr ? _graphModel->hasVisualisationInfo() : false;
}

QVariantMap Document::visualisationInfoAtIndex(int index) const
{
    QVariantMap map;
    QVariantList numericValues;
    QVariantList stringValues;

    map.insert(QStringLiteral("alertType"), static_cast<int>(AlertType::None));
    map.insert(QStringLiteral("alertText"), "");
    map.insert(QStringLiteral("minimumNumericValue"), 0.0);
    map.insert(QStringLiteral("maximumNumericValue"), 1.0);
    map.insert(QStringLiteral("mappedMinimumNumericValue"), 0.0);
    map.insert(QStringLiteral("mappedMaximumNumericValue"), 1.0);
    map.insert(QStringLiteral("hasNumericRange"), true);
    map.insert(QStringLiteral("numericValues"), numericValues);
    map.insert(QStringLiteral("stringValues"), stringValues);
    map.insert(QStringLiteral("numApplications"), 1);

    if(_graphModel == nullptr)
        return map;

    const auto& visualisationInfo = _graphModel->visualisationInfoAtIndex(index);

    map.insert(QStringLiteral("minimumNumericValue"), visualisationInfo.statistics()._min);
    map.insert(QStringLiteral("maximumNumericValue"), visualisationInfo.statistics()._max);
    map.insert(QStringLiteral("mappedMinimumNumericValue"), visualisationInfo.mappedMinimum());
    map.insert(QStringLiteral("mappedMaximumNumericValue"), visualisationInfo.mappedMaximum());
    map.insert(QStringLiteral("hasNumericRange"), visualisationInfo.statistics()._range > 0.0);
    map.insert(QStringLiteral("numApplications"), visualisationInfo.numApplications());

    const auto& numericValuesVector = visualisationInfo.statistics()._values;
    numericValues.reserve(static_cast<int>(numericValuesVector.size()));
    for(auto value : numericValuesVector)
        numericValues.append(value);

    map.insert(QStringLiteral("numericValues"), numericValues);

    const auto& stringValuesVector = visualisationInfo.stringValues();
    stringValues.reserve(static_cast<int>(stringValuesVector.size()));
    for(const auto& stringValue : stringValuesVector)
        stringValues.append(stringValue);

    map.insert(QStringLiteral("stringValues"), stringValues);

    auto alerts = visualisationInfo.alerts();

    if(alerts.empty())
        return map;

    std::sort(alerts.begin(), alerts.end(),
    [](auto& a, auto& b)
    {
        return a._type > b._type;
    });

    auto& alert = alerts.at(0);

    map.insert(QStringLiteral("alertType"), static_cast<int>(alert._type));
    map.insert(QStringLiteral("alertText"), alert._text);

    return map;
}

// NOLINTNEXTLINE readability-convert-member-functions-to-static
QVariantMap Document::parseVisualisation(const QString& visualisation) const
{
    VisualisationConfigParser p;
    if(p.parse(visualisation))
        return p.result().asVariantMap();

    return {};
}

QVariantMap Document::visualisationDefaultParameters(int valueType,
                                                     const QString& channelName) const
{
    return _graphModel != nullptr ? _graphModel->visualisationDefaultParameters(
        static_cast<ValueType>(valueType), channelName) : QVariantMap();
}

bool Document::visualisationIsValid(const QString& visualisation) const
{
    return _graphModel != nullptr ? _graphModel->visualisationIsValid(visualisation) : false;
}

void Document::setVisualisation(int index, const QString& visualisation)
{
    Q_ASSERT(index >= 0 && index < _visualisationsFromUI.count());
    _visualisationsFromUI[index] = visualisation;
    emit visualisationsChanged();
}

void Document::removeVisualisation(int index)
{
    Q_ASSERT(index >= 0 && index < _visualisationsFromUI.count());
    _visualisationsFromUI.removeAt(index);
    emit visualisationsChanged();
}

// This tests two visualisation lists to determine if replacing one with the
// other would actually result in a different visualisation
static bool visualisationsDiffer(const QStringList& a, const QStringList& b)
{
    if(a.length() != b.length())
        return true;

    VisualisationConfigParser p;

    for(int i = 0; i < a.length(); i++)
    {
        VisualisationConfig ai, bi;

        if(p.parse(a[i]))
            ai = p.result();

        if(p.parse(b[i]))
            bi = p.result();

        if(ai != bi)
            return true;
    }

    return false;
}

void Document::moveVisualisation(int from, int to)
{
    if(_graphModel == nullptr)
        return;

    if(from < 0 || from >= _graphTransforms.size())
        return;

    if(to < 0 || to >= _graphTransforms.size())
        return;

    QStringList newVisualisations = _visualisations;
    newVisualisations.move(from, to);

    _commandManager.execute(ExecutePolicy::Add,
        std::make_unique<ApplyVisualisationsCommand>(
        _graphModel.get(), this,
        _visualisations, newVisualisations));
}

void Document::update(QStringList newGraphTransforms,
    QStringList newVisualisations, // NOLINT performance-unnecessary-value-param
    bool replaceLatestCommand)
{
    if(_graphModel == nullptr)
        return;

    ICommandPtrsVector commands;

    auto uiGraphTransforms = graphTransformConfigurationsFromUI();
    int newGraphTransformIndex = -1;

    if(!newGraphTransforms.empty())
    {
        int index = 0;

        for(const auto& newGraphTransform : std::as_const(newGraphTransforms))
        {
            if(!transformIsPinned(newGraphTransform))
            {
                // Insert before any existing pinned transforms
                index = 0;
                while(index < uiGraphTransforms.size() && !transformIsPinned(uiGraphTransforms.at(index)))
                    index++;

                uiGraphTransforms.insert(index, newGraphTransform);
            }
            else
            {
                uiGraphTransforms.append(newGraphTransform);
                index = uiGraphTransforms.size() - 1;
            }
        }

        if(newGraphTransforms.size() == 1)
            newGraphTransformIndex = index;
    }

    bool transformsValid = std::all_of(uiGraphTransforms.begin(), uiGraphTransforms.end(),
    [this](const auto& transform)
    {
        return _graphModel->graphTransformIsValid(transform);
    });

    if(transformsValid)
    {
        if(transformsDiffer(_graphTransforms, uiGraphTransforms))
        {
            commands.emplace_back(std::make_unique<ApplyTransformsCommand>(
                _graphModel.get(), _selectionManager.get(), this,
                _graphTransforms, uiGraphTransforms));
        }
        else
        {
            auto previousGraphTransforms = _graphTransforms;

            commands.emplace_back(std::make_unique<Command>(
                Command::CommandDescription
                {
                    QStringLiteral("Apply Transform Flags"),
                    QStringLiteral("Applying Transform Flags")
                },
                [this, uiGraphTransforms](Command&)         { setTransforms(uiGraphTransforms); },
                [this, previousGraphTransforms](Command&)   { setTransforms(previousGraphTransforms); }));
        }
    }
    else
        setTransforms(_graphTransforms);

    auto uiVisualisations = _visualisationsFromUI;

    if(!newVisualisations.empty())
    {
        _graphModel->clearVisualisationInfos();
        uiVisualisations.append(newVisualisations);
    }

    if(visualisationsDiffer(_visualisations, uiVisualisations))
    {
        commands.emplace_back(std::make_unique<ApplyVisualisationsCommand>(
            _graphModel.get(), this,
            _visualisations, uiVisualisations, newGraphTransformIndex));
    }
    else
        setVisualisations(uiVisualisations);

    ExecutePolicy policy = replaceLatestCommand ?
        ExecutePolicy::Replace : ExecutePolicy::Add;

    if(commands.size() > 1)
    {
        _commandManager.execute(policy, std::move(commands),
            {
                tr("Apply Transforms and Visualisations"),
                tr("Applying Transforms and Visualisations")
            });
    }
    else if(commands.size() == 1)
        _commandManager.execute(policy, std::move(commands.front()));
}

QVariantMap Document::layoutSetting(const QString& name) const
{
    QVariantMap map;

    const auto* setting = _layoutThread->setting(name);
    if(setting != nullptr)
    {
        map.insert(QStringLiteral("name"), setting->name());
        map.insert(QStringLiteral("displayName"), setting->displayName());
        map.insert(QStringLiteral("description"), setting->description());
        map.insert(QStringLiteral("value"), setting->value());
        map.insert(QStringLiteral("normalisedValue"), setting->normalisedValue());
        map.insert(QStringLiteral("minimumValue"), setting->minimumValue());
        map.insert(QStringLiteral("maximumValue"), setting->maximumValue());
        map.insert(QStringLiteral("defaultValue"), setting->defaultValue());
    }

    return map;
}

void Document::setLayoutSettingValue(const QString& name, float value)
{
    _layoutThread->setSettingValue(name, value);
}

void Document::setLayoutSettingNormalisedValue(const QString& name, float normalisedValue)
{
    _layoutThread->setSettingNormalisedValue(name, normalisedValue);
}

void Document::resetLayoutSettingValue(const QString& name)
{
    _layoutThread->resetSettingValue(name);
}

float Document::nodeSize() const
{
    if(_graphModel == nullptr)
        return u::pref(QStringLiteral("visuals/defaultNormalNodeSize")).toFloat();

    return _graphModel->nodeSize();
}

void Document::setNodeSize(float nodeSize)
{
    if(_graphModel == nullptr)
        return;

    if(nodeSize != _graphModel->nodeSize())
    {
        _graphModel->setNodeSize(nodeSize);
        emit nodeSizeChanged();
        setSaveRequired();
    }
}

void Document::resetNodeSize()
{
    auto defaultNormalNodeSize = u::pref(QStringLiteral("visuals/defaultNormalNodeSize")).toFloat();
    setNodeSize(defaultNormalNodeSize);
}

float Document::edgeSize() const
{
    if(_graphModel == nullptr)
        return u::pref(QStringLiteral("visuals/defaultNormalEdgeSize")).toFloat();

    return _graphModel->edgeSize();
}

void Document::setEdgeSize(float edgeSize)
{
    if(_graphModel == nullptr)
        return;

    if(edgeSize != _graphModel->edgeSize())
    {
        _graphModel->setEdgeSize(edgeSize);
        emit edgeSizeChanged();
        setSaveRequired();
    }
}

void Document::resetEdgeSize()
{
    auto defaultNormalEdgeSize = u::pref(QStringLiteral("visuals/defaultNormalEdgeSize")).toFloat();
    setEdgeSize(defaultNormalEdgeSize);
}

void Document::cancelCommand()
{
    if(!_loadComplete && _graphFileParserThread != nullptr)
    {
        _graphFileParserThread->cancel();

        // If the loader isn't complete, but has made it past the parsing stage proper,
        // simply cancelling the parser won't cancel the (potentially long running)
        // transform build stage, so we need to do that too
        _graphModel->cancelTransformBuild();
    }
    else
        _commandManager.cancel();
}

void Document::writeTableView2ToFile(QObject* tableView, const QUrl& fileUrl, const QString& extension)
{
    auto columnCount = QQmlProperty::read(tableView, QStringLiteral("columns")).toInt();

    QVariant columnNamesVariant;
    QMetaObject::invokeMethod(tableView, "visibleColumnNames", Q_RETURN_ARG(QVariant, columnNamesVariant));
    auto columnNames = columnNamesVariant.toStringList();

    QString localFileName = fileUrl.toLocalFile();
    if(!QFile(localFileName).open(QIODevice::ReadWrite))
    {
        QMessageBox::critical(nullptr, tr("File Error"),
            QString(tr("The file '%1' cannot be opened for writing. Please ensure "
            "it is not open in another application and try again.")).arg(localFileName));
        return;
    }

    _commandManager.executeOnce(
    [=](Command&)
    {
        QFile file(localFileName);

        if(!file.open(QIODevice::ReadWrite|QIODevice::Truncate))
        {
            // We should never get here normally, since this check has already been performed
            qDebug() << "Can't open" << localFileName << "for writing.";
            return;
        }

        auto csvEscapedString = [](const QString& string)
        {
            if(string.contains(QRegularExpression(QStringLiteral(R"([",])"))))
            {
                QString escaped = string;

                // Encode " as ""
                escaped.replace(QStringLiteral(R"(")"), QStringLiteral(R"("")"));

                return QStringLiteral(R"("%1")").arg(escaped);
            }

            return string;
        };

        auto tsvEscapedString = [](const QString& string)
        {
            QString escaped = string;

            // "The IANA standard for TSV achieves simplicity
            // by simply disallowing tabs within fields."
            return escaped.replace(QStringLiteral("\t"), QString());
        };

        std::function<QString(const QString&)> escapedString = csvEscapedString;
        QString separator = QStringLiteral(",");

        if(extension == QStringLiteral("tsv"))
        {
            escapedString = tsvEscapedString;
            separator = QStringLiteral("\t");
        }

        QString rowString;
        for(const auto& columnName : columnNames)
        {
            if(!rowString.isEmpty())
                rowString.append(separator);

            rowString.append(escapedString(columnName));
        }

        QTextStream stream(&file);
        stream << rowString << "\r\n";

        auto rowCount = QQmlProperty::read(tableView, QStringLiteral("rows")).toInt();
        auto* model = qvariant_cast<QAbstractItemModel*>(QQmlProperty::read(tableView, QStringLiteral("model")));
        if(model != nullptr)
        {
            for(int row = 0; row < rowCount; row++)
            {
                rowString.clear();
                for(int column = 0; column < columnCount; column++)
                {
                    if(!rowString.isEmpty())
                        rowString.append(separator);

                    auto value = model->data(model->index(row, column));

                    auto valueString = value.toString();

                    if(value.type() == QVariant::String)
                        rowString.append(escapedString(valueString));
                    else
                        rowString.append(valueString);
                }

                stream << rowString << "\r\n";
            }
        }
    }, tr("Exporting Table"));
}

void Document::writeTableViewToFile(QObject* tableView, const QUrl& fileUrl, const QString& extension)
{
    // We have to do this part on the same thread as the caller, because we can't invoke
    // methods across threads; hopefully it's relatively quick
    QStringList columnRoles;
    auto columnCount = QQmlProperty::read(tableView, QStringLiteral("columnCount")).toInt();
    for(int i = 0; i < columnCount; i++)
    {
        QVariant columnVariant;
        QMetaObject::invokeMethod(tableView, "getColumn",
                Q_RETURN_ARG(QVariant, columnVariant),
                Q_ARG(QVariant, i));
        auto* tableViewColumn = qvariant_cast<QObject*>(columnVariant);

        if(tableViewColumn != nullptr && QQmlProperty::read(tableViewColumn, QStringLiteral("visible")).toBool())
            columnRoles.append(QQmlProperty::read(tableViewColumn, QStringLiteral("role")).toString());
    }

    QString localFileName = fileUrl.toLocalFile();
    if(!QFile(localFileName).open(QIODevice::ReadWrite))
    {
        QMessageBox::critical(nullptr, tr("File Error"),
            QString(tr("The file '%1' cannot be opened for writing. Please ensure "
            "it is not open in another application and try again.")).arg(localFileName));
        return;
    }

    _commandManager.executeOnce(
    [=](Command&)
    {
        QFile file(localFileName);

        if(!file.open(QIODevice::ReadWrite|QIODevice::Truncate))
        {
            // We should never get here normally, since this check has already been performed
            qDebug() << "Can't open" << localFileName << "for writing.";
            return;
        }

        auto csvEscapedString = [](const QString& string)
        {
            if(string.contains(QRegularExpression(QStringLiteral(R"([",])"))))
            {
                QString escaped = string;

                // Encode " as ""
                escaped.replace(QStringLiteral(R"(")"), QStringLiteral(R"("")"));

                return QStringLiteral(R"("%1")").arg(escaped);
            }

            return string;
        };

        auto tsvEscapedString = [](const QString& string)
        {
            QString escaped = string;

            // "The IANA standard for TSV achieves simplicity
            // by simply disallowing tabs within fields."
            return escaped.replace(QStringLiteral("\t"), QString());
        };

        std::function<QString(const QString&)> escapedString = csvEscapedString;
        QString separator = QStringLiteral(",");

        if(extension == QStringLiteral("tsv"))
        {
            escapedString = tsvEscapedString;
            separator = QStringLiteral("\t");
        }

        QString rowString;
        for(const auto& columnRole : columnRoles)
        {
            if(!rowString.isEmpty())
                rowString.append(separator);

            rowString.append(escapedString(columnRole));
        }

        QTextStream stream(&file);
        stream << rowString << "\r\n";

        auto rowCount = QQmlProperty::read(tableView, QStringLiteral("rowCount")).toInt();
        auto* model = qvariant_cast<QAbstractItemModel*>(QQmlProperty::read(tableView, QStringLiteral("model")));
        if(model != nullptr)
        {
            for(int row = 0; row < rowCount; row++)
            {
                rowString.clear();
                for(const auto& columnRole : columnRoles)
                {
                    if(!rowString.isEmpty())
                        rowString.append(separator);

                    auto value = model->data(model->index(row, 0),
                        model->roleNames().key(columnRole.toUtf8(), -1));
                    auto valueString = value.toString();

                    if(value.type() == QVariant::String)
                        rowString.append(escapedString(valueString));
                    else
                        rowString.append(valueString);
                }

                stream << rowString << "\r\n";
            }
        }
    }, tr("Exporting Table"));
}

void Document::copyTableViewCopyToClipboard(QObject* tableView, int column)
{
    auto columnCount = QQmlProperty::read(tableView, QStringLiteral("columns")).toInt();

    if(column < 0 || column >= columnCount)
    {
        qDebug() << "Document::copyTableViewCopyToClipboard: requested column exceeds column count";
        return;
    }

    auto* model = qvariant_cast<QAbstractItemModel*>(QQmlProperty::read(tableView, QStringLiteral("model")));
    if(model == nullptr)
    {
        qDebug() << "Document::copyTableViewCopyToClipboard: null model";
        return;
    }

    auto rowCount = QQmlProperty::read(tableView, QStringLiteral("rows")).toInt();

    QString text;

    for(int row = 0; row < rowCount; row++)
    {
        auto value = model->data(model->index(row, column));
        text.append(QStringLiteral("%1\n").arg(value.toString()));
    }

    QApplication::clipboard()->setText(text);
}

void Document::addBookmark(const QString& name)
{
    if(_selectionManager == nullptr)
        return;

    _bookmarks.insert({name, _selectionManager->selectedNodes()});
    emit bookmarksChanged();
    setSaveRequired();
}

void Document::removeBookmarks(const QStringList& names)
{
    bool removed = false;

    for(const auto& name : names)
    {
        const auto it = _bookmarks.find(name);
        if(it != _bookmarks.end())
        {
            _bookmarks.erase(it);
            removed = true;
        }
    }

    if(removed)
    {
        emit bookmarksChanged();
        setSaveRequired();
    }
}

void Document::renameBookmark(const QString& from, const QString& to)
{
    if(u::containsKey(_bookmarks, from) && !u::containsKey(_bookmarks, to))
    {
        const auto it = _bookmarks.find(from);
        std::swap(_bookmarks[to], it->second);
        _bookmarks.erase(it);
        emit bookmarksChanged();
        setSaveRequired();
    }
}

void Document::gotoBookmark(const QString& name)
{
    if(_selectionManager != nullptr && u::containsKey(_bookmarks, name))
    {
        std::vector<NodeId> nodeIds;
        for(auto nodeId : _graphModel->graph().nodeIds())
        {
            if(_graphModel->graph().typeOf(nodeId) != MultiElementType::Tail)
                nodeIds.emplace_back(nodeId);
        }

        nodeIds = u::setIntersection(nodeIds, _bookmarks[name]);
        selectAndFocusNodes(nodeIds);
    }
}

void Document::gotoAllBookmarks()
{
    if(_selectionManager != nullptr)
    {
        NodeIdSet bookmarkedNodeIds;

        for(const auto& bookmark : _bookmarks)
            bookmarkedNodeIds.insert(bookmark.second.begin(), bookmark.second.end());

        std::vector<NodeId> nodeIds;
        for(auto nodeId : _graphModel->graph().nodeIds())
        {
            if(_graphModel->graph().typeOf(nodeId) != MultiElementType::Tail)
                nodeIds.emplace_back(nodeId);
        }

        nodeIds = u::setIntersection(nodeIds, bookmarkedNodeIds);
        selectAndFocusNodes(nodeIds);
    }
}

void Document::dumpGraph()
{
    _graphModel->graph().dumpToQDebug(2);
}

void Document::performEnrichment(const QString& selectedAttributeA, const QString& selectedAttributeB)
{
    auto* tableModel = new EnrichmentTableModel(this);

    commandManager()->executeOnce(
    [this, selectedAttributeA, selectedAttributeB, tableModel](Command& command) mutable
    {
        auto result = EnrichmentCalculator::overRepAgainstEachAttribute(
            selectedAttributeA, selectedAttributeB, graphModel(), command);

        tableModel->setTableData(result, selectedAttributeA, selectedAttributeB);

        executeOnMainThreadAndWait([this, tableModel]
        {
            _enrichmentTableModels.append(QVariant::fromValue(tableModel));
        });

        emit enrichmentTableModelsChanged();
        emit enrichmentAnalysisComplete();

        return true;
    }, tr("Enrichment Analysis"));
}

void Document::removeEnrichmentResults(int index)
{
    _enrichmentTableModels.removeAt(index);
    emit enrichmentTableModelsChanged();
}

void Document::saveNodePositionsToFile(const QUrl& fileUrl)
{
    QString localFileName = fileUrl.toLocalFile();
    if(!QFile(localFileName).open(QIODevice::ReadWrite))
    {
        QMessageBox::critical(nullptr, tr("File Error"),
            QString(tr("The file '%1' cannot be opened for writing. Please ensure "
            "it is not open in another application and try again.")).arg(localFileName));
        return;
    }

    commandManager()->executeOnce(
    [this, localFileName](Command& command)
    {
        json positions;

        uint64_t i = 0;
        for(auto nodeId : _graphModel->graph().nodeIds())
        {
            auto name = _graphModel->nodeNames().at(nodeId);
            auto v = _graphModel->nodePositions().get(nodeId);

            positions.push_back(
            {
                {"id", static_cast<int>(nodeId)},
                {"name", name.toStdString()},
                {"position", {v.x(), v.y(), v.z()}}
            });

            command.setProgress(static_cast<int>((i++ * 100u) /
                static_cast<uint64_t>(_graphModel->graph().numNodes())));
        }

        command.setProgress(-1);

        QFile file(localFileName);

        if(!file.open(QIODevice::ReadWrite|QIODevice::Truncate))
        {
            // We should never get here normally, since this check has already been performed
            qDebug() << "Can't open" << localFileName << "for writing.";
            return;
        }

        file.write(QByteArray::fromStdString(positions.dump()));
    }, tr("Exporting Node Positions"));
}

// NOLINTNEXTLINE readability-make-member-function-const
void Document::cloneAttribute(const QString& sourceAttributeName, const QString& newAttributeName)
{
    if(busy())
        return;

    _commandManager.execute(ExecutePolicy::Add,
        std::make_unique<CloneAttributeCommand>(_graphModel.get(), sourceAttributeName, newAttributeName));
}

void Document::editAttribute(const QString& attributeName, const AttributeEdits& edits)
{
    if(busy())
        return;

    _commandManager.execute(ExecutePolicy::Add,
        std::make_unique<EditAttributeCommand>(_graphModel.get(), attributeName, edits));
}

// NOLINTNEXTLINE readability-make-member-function-const
void Document::removeAttributes(const QStringList& attributeNames)
{
    if(busy())
        return;

    _commandManager.execute(ExecutePolicy::Add,
        std::make_unique<RemoveAttributesCommand>(_graphModel.get(), attributeNames));
}

// NOLINTNEXTLINE readability-make-member-function-const
void Document::importAttributesFromTable(const QString& keyAttributeName,
    std::shared_ptr<TabularData> data, // NOLINT performance-unnecessary-value-param
    int keyColumnIndex, const std::vector<int>& importColumnIndices, bool replace)
{
    if(busy())
        return;

    _commandManager.execute(ExecutePolicy::Add,
        std::make_unique<ImportAttributesCommand>(_graphModel.get(),
        keyAttributeName, data.get(), keyColumnIndex,
        importColumnIndices, replace));
}

QString Document::graphSizeSummary() const
{
    if(_graphModel == nullptr)
        return {};

    QString text;

    text += QStringLiteral("Mutable Graph Nodes: %1 Edges: %2\n")
        .arg(_graphModel->mutableGraph().numNodes())
        .arg(_graphModel->mutableGraph().numEdges());

    text += QStringLiteral("Transformed Graph Nodes: %1 Edges: %2 Components: %3")
        .arg(_graphModel->graph().numNodes())
        .arg(_graphModel->graph().numEdges())
        .arg(_graphModel->graph().numComponents());

    return text;
}

QString Document::commandStackSummary() const
{
    return _commandManager.commandStackSummary();
}

static_block
{
    qmlRegisterType<Document>(
        APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "Document");
}
