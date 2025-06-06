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

#include "document.h"

#include "app/application.h"
#include "app/preferences.h"
#include "app/limitconstants.h"

#include "app/attributes/enrichmentcalculator.h"

#include "shared/plugins/iplugin.h"
#include "shared/utils/flags.h"
#include "shared/utils/color.h"
#include "shared/utils/string.h"
#include "shared/loading/tabulardata.h"
#include "shared/loading/userelementdata.h"

#include "app/graph/mutablegraph.h"
#include "app/graph/graphmodel.h"

#include "app/loading/parserthread.h"
#include "app/loading/nativeloader.h"
#include "app/loading/nativesaver.h"
#include "app/loading/isaver.h"

#include "app/layout/forcedirectedlayout.h"
#include "app/layout/layout.h"
#include "app/layout/collision.h"

#include "app/commands/applytransformscommand.h"
#include "app/commands/applyvisualisationscommand.h"
#include "app/commands/deletenodescommand.h"
#include "app/commands/cloneattributecommand.h"
#include "app/commands/editattributecommand.h"
#include "app/commands/removeattributescommand.h"
#include "app/commands/importattributescommand.h"
#include "app/commands/selectnodescommand.h"

#include "app/transform/graphtransform.h"
#include "app/transform/graphtransformconfigparser.h"
#include "app/ui/visualisations/visualisationinfo.h"
#include "app/ui/visualisations/visualisationconfigparser.h"

#include "app/attributes/conditionfncreator.h"
#include "app/attributes/attributeedits.h"

#include "app/ui/searchmanager.h"
#include "app/ui/selectionmanager.h"

#include "availableattributesmodel.h"
#include "availabletransformsmodel.h"

#include "../crashhandler.h"

#include <json_helper.h>
#include <numeric>
#include <thread>
#include <chrono>

#include <QQmlProperty>
#include <QMetaObject>
#include <QFile>
#include <QAbstractItemModel>
#include <QMessageBox>
#include <QApplication>
#include <QElapsedTimer>
#include <QVariantList>
#include <QVector>
#include <QClipboard>
#include <QQmlEngine>
#include <QThread>

using namespace Qt::Literals::StringLiterals;

QColor Document::contrastingColorForBackground()
{
    auto backColor = u::pref(u"visuals/backgroundColor"_s).value<QColor>();
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
    if(!_graphDisplay->initialised())
        return true;

    return commandInProgress() || graphChanging() ||
        _graphDisplay->updating() || _graphDisplay->transitioning() ||
        _graphDisplay->interacting();
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

        qDebug().noquote() << u"busy %1%2%3%4%5%6"_s.arg(
            (commandInProgress() ?                  "C" : "."),
            (graphChanging() ?                      "G" : "."),
            (_graphDisplay->updating() ?            "U" : "."),
            (_graphDisplay->interacting() ?         "I" : "."),
            (_graphDisplay->transitioning() ?       "T" : "."),
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

    if(!_loadComplete)
    {
        if(!_loadPhase.isEmpty())
            return QString(tr("Loading %1 (%2)").arg(_title, _loadPhase));

        return QString(tr("Loading %1").arg(_title));
    }

    if(!_commandManager.commandPhase().isEmpty())
        return QString(tr("%1 (%2)")).arg(_commandManager.commandVerb(), _commandManager.commandPhase());

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

std::vector<LayoutSetting> Document::layoutSettings() const
{
    if(_layoutThread != nullptr)
        return _layoutThread->settings();

    return {};
}

void Document::updateLayoutDimensionality()
{
    if(_graphDisplay == nullptr)
        return;

    auto newDimensionality = _graphDisplay->projection() == Projection::TwoDee ?
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
    if(_graphDisplay == nullptr)
    {
        // Running is returned here for the case where we're saving in headless mode
        return LayoutPauseState::Running;
    }

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
    return !busy() && !_graphDisplay->viewIsReset();
}

bool Document::canEnterOverviewMode() const
{
    return !busy() && _graphDisplay->canEnterOverviewMode();
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

    _visualisationsFromUI = visualisations;
    emit visualisationsChanged();

    setSaveRequired();
}

void Document::refreshVisualisations()
{
    // This shouldn't be called on the main thread as GraphModel::buildVisualisations
    // can potentially take some time to complete
    Q_ASSERT(QThread::currentThread() != QApplication::instance()->thread());

    _graphModel->buildVisualisations(_visualisations);

    executeOnMainThreadAndWait([this]
    {
        setVisualisations(_visualisations);
    }, u"Document refreshVisualisations"_s);
}


float Document::fps() const
{
    if(_graphDisplay != nullptr)
        return _graphDisplay->fps();

    return 0.0f;
}

QObject* Document::pluginInstance()
{
    // This will return nullptr if _pluginInstance is not a QObject, which is
    // allowed, but means that the UI can't interact with it
    return dynamic_cast<QObject*>(_pluginInstance.get());
}

QString Document::pluginQmlModule() const
{
    return _graphModel != nullptr ? _graphModel->pluginQmlModule() : QString();
}

QStringList Document::bookmarks() const
{
    QStringList list;
    list.reserve(static_cast<int>(_bookmarks.size()));

    for(const auto& name : u::keysFor(_bookmarks))
        list.append(name);

    std::sort(list.begin(), list.end(),
    [](const auto& a, const auto& b)
    {
        return u::numericCompareCaseInsensitive(a, b) < 0;
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
    return p.result().isFlagSet(u"pinned"_s);
}

static QStringList sortedTransforms(QStringList transforms)
{
    // Sort so that the pinned transforms go last
    std::stable_sort(transforms.begin(), transforms.end(),
    [](const QString& a, const QString& b)
    {
        const bool aPinned = transformIsPinned(a);
        const bool bPinned = transformIsPinned(b);

        if(aPinned && !bPinned)
            return false;

        if(!aPinned && bPinned)
            return true;

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
    {
        setFailureReason(tr("The plugin %1 could not be found.").arg(pluginName));
        emit failureReasonChanged();
        emit loadComplete(url, false);
        return false;
    }

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
        const bool hasSignal = pluginInstanceQObject->metaObject()->indexOfSignal(signature) >= 0;

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

    connect(_graphModel.get(), &GraphModel::attributesChanged, this, &Document::attributesChanged); // clazy:exclude=connect-non-signal
    connect(_graphModel.get(), &GraphModel::attributesChanged, this, &Document::setSaveRequired); // clazy:exclude=connect-non-signal

    emit pluginInstanceChanged();

    if(parser == nullptr)
    {
        // If we don't yet have a parser, we need to ask the plugin for one
        parser = _pluginInstance->parserForUrlTypeName(type);

        if(parser == nullptr)
        {
            setFailureReason(tr("The plugin %1 does not provide a parser for %2. "
                "This is probably a bug, please report it to the developers.")
                .arg(pluginName, type));

            emit failureReasonChanged();
            emit loadComplete(url, false);
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

            // Don't waste time building transforms if we're not displaying the graph
            if(_graphDisplay != nullptr)
            {
                _graphModel->buildTransforms(_graphTransforms, completedLoader);

                if(completedParser->cancelled())
                    return;

                _graphModel->buildVisualisations(_visualisations);
            }

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
            setLog(log() + u"\n\nNodes: %1 Edges: %2"_s
                .arg(graph.numNodes()).arg(graph.numEdges()));

            _graphTransforms = _graphModel->transformsWithMissingParametersSetToDefault(
                sortedTransforms(_pluginInstance->defaultTransforms()));
            _visualisations = _pluginInstance->defaultVisualisations();

            if(_graphDisplay != nullptr)
                _graphModel->buildTransforms(_graphTransforms, completedParser);

            for(auto& visualisation : _visualisations)
            {
                VisualisationConfigParser p;
                const bool success = p.parse(visualisation);
                Q_ASSERT(success);

                if(!p.result()._parameters.empty())
                    continue;

                // If the visualisation was not supplied with parameters, apply the defaults
                const auto& attributeName = p.result()._attributeName;
                const ValueType valueType = _graphModel->attributeExists(attributeName) ?
                    _graphModel->attributeValueByName(attributeName).valueType() :
                    _graphModel->valueTypeOfTransformAttributeName(attributeName);

                const auto& channelName = p.result()._channelName;

                auto defaultParameters = _graphModel->visualisationDefaultParameters(valueType, channelName);

                if(!defaultParameters.isEmpty())
                {
                    visualisation += u" with"_s;

                    const auto& parameterKeys = defaultParameters.keys();
                    for(const auto& key : parameterKeys)
                    {
                        auto value = u::escapeQuotes(defaultParameters.value(key).toString());
                        visualisation += u" %1 = \"%2\""_s.arg(key, value);
                    }
                }
            }

            if(_graphDisplay != nullptr)
                _graphModel->buildVisualisations(_visualisations);
        });
    }

    connect(_graphFileParserThread.get(), &ParserThread::progressChanged, this, &Document::onLoadProgressChanged);
    connect(_graphFileParserThread.get(), &ParserThread::phaseChanged, this, &Document::onLoadPhaseChanged);
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
#ifdef Q_OS_WASM
        // Under wasm, fileUrl will be a temporary file name
        auto filename = tr("file");
#else
        auto filename = fileUrl.fileName();
#endif

        _commandManager.executeOnce(
        [this, factory, fileUrl, saverName, uiData, pluginUiData](Command& command) mutable
        {
            auto saver = factory->create(fileUrl, this, _pluginInstance.get(), uiData, pluginUiData);
            saver->setProgressFn([&command](int percentage){ command.setProgress(percentage); });
            saver->setPhaseFn([&command](const QString& phase){ command.setPhase(phase); });
            const bool success = saver->save();
            emit saveComplete(success, fileUrl, saverName);
            return success;
        },
        {
            tr("Save %1").arg(filename),
            tr("Saving %1").arg(filename),
            tr("Saved %1").arg(filename)
        });

        _saveRequired = false;
        emit saveRequiredChanged();
    }
    else
    {
        QMessageBox::critical(nullptr, tr("Save Error"), u"%1 %2"_s
            .arg(tr("Unable to find registered saver with name:"), saverName));
    }
}

void Document::onPreferenceChanged(const QString& key, const QVariant&)
{
    if(key == u"visuals/backgroundColor"_s)
        emit contrastingColorChanged();
    else if(key == u"visuals/showEdgeText"_s)
    {
        // showEdgeText affects the warning state of TextVisualisationChannel
        _commandManager.executeOnce([this](Command&) { refreshVisualisations(); });
    }
}

void Document::onLoadProgressChanged(int percentage)
{
    _loadProgress = percentage;
    emit commandProgressChanged();
}

void Document::onLoadPhaseChanged(const QString& phase)
{
    _loadPhase = phase;
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

    if(_graphDisplay == nullptr)
    {
        _loadComplete = true;

        connect(&_commandManager, &CommandManager::commandProgressChanged, this, &Document::commandProgressChanged);
        connect(&_commandManager, &CommandManager::commandPhaseChanged, this, &Document::commandVerbChanged);
        connect(&_commandManager, &CommandManager::commandVerbChanged, this, &Document::commandVerbChanged);

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
        _layoutThread->setNodePositions(*_startingNodePositions);
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
    emit textSizeChanged();

    // Load UI saved data
    if(_uiData.size() > 0)
        emit uiDataChanged(_uiData);

    // This causes the plugin UI to be loaded
    emit pluginQmlModuleChanged(_pluginUiData, _pluginUiDataVersion);

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

    _graphDisplay->initialise(_graphModel.get(), &_commandManager, _selectionManager.get());

    connect(_graphDisplay, &GraphDisplay::initialisedChanged, this, &Document::maybeEmitBusyChanged, Qt::QueuedConnection);
    connect(_graphDisplay, &GraphDisplay::updatingChanged, this, &Document::maybeEmitBusyChanged, Qt::QueuedConnection);
    connect(_graphDisplay, &GraphDisplay::interactingChanged, this, &Document::maybeEmitBusyChanged, Qt::QueuedConnection);
    connect(_graphDisplay, &GraphDisplay::transitioningChanged, this, &Document::maybeEmitBusyChanged, Qt::QueuedConnection);
    connect(_graphDisplay, &GraphDisplay::viewIsResetChanged, this, &Document::canResetViewChanged);
    connect(_graphDisplay, &GraphDisplay::canEnterOverviewModeChanged, this, &Document::canEnterOverviewModeChanged);
    connect(_graphDisplay, &GraphDisplay::fpsChanged, this, &Document::fpsChanged);
    connect(_graphDisplay, &GraphDisplay::visibleComponentIndexChanged, this, &Document::numInvisibleNodesSelectedChanged);

    connect(&_commandManager, &CommandManager::started, this, &Document::maybeEmitBusyChanged, Qt::DirectConnection);
    connect(&_commandManager, &CommandManager::started, this, &Document::commandInProgressChanged);

    connect(&_commandManager, &CommandManager::started, _graphDisplay, &GraphDisplay::commandsStarted);
    connect(&_commandManager, &CommandManager::finished, _graphDisplay, &GraphDisplay::commandsFinished);

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
    connect(&_commandManager, &CommandManager::commandPhaseChanged, this, &Document::commandVerbChanged);
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
        _commandManager.clearPhase();

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

    connect(_layoutThread.get(), &LayoutThread::executed, _graphDisplay, &GraphDisplay::onLayoutChanged);

    connect(_graphModel.get(), &GraphModel::visualsChanged, this, &Document::hasValidEdgeTextVisualisationChanged); // clazy:exclude=connect-non-signal
    connect(_graphModel.get(), &GraphModel::rebuildRequired, // clazy:exclude=connect-non-signal
    [this](bool transforms, bool visualisations)
    {
        ICommandPtrsVector commands;

        if(transforms)
        {
            commands.emplace_back(
                std::make_unique<ApplyTransformsCommand>(
                _graphModel.get(), this,
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
        const bool graphChangingWillChange = !_graphChanging;
        _graphChanging = true;

        if(graphChangingWillChange)
            emit graphChangingChanged();

        maybeEmitBusyChanged();
    });

    connect(&_graphModel->graph(), &Graph::graphChanged, [this]
    (const Graph*, bool changeOccurred)
    {
        const bool graphChangingWillChange = _graphChanging;
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
        }, u"Document (mutable) graphChanged"_s);

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
        const bool nodesSelected = _selectionManager->selectAllNodes();
        command.setPastParticiple(_selectionManager->numNodesSelectedAsString());
        return nodesSelected;
    }, {tr("Select All"), tr("Selecting All"), tr("Selected All")});
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
        auto componentId = _graphDisplay->focusedComponentId();
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
            {tr("Select None"), tr("Selecting None"), tr("Selected None")});
    }
}

static NodeIdSet nodeIdSetFor(QmlNodeId nodeId, bool add, const SelectionManager& selectionManager)
{
    NodeIdSet nodeIdSet;

    if(add)
        nodeIdSet = selectionManager.selectedNodes();

    if(!nodeId.isNull())
        nodeIdSet.emplace(nodeId);

    return nodeIdSet;
}

void Document::selectSources(QmlNodeId qmlNodeId, bool add)
{
    if(busy())
        return;

    NodeIdSet nodeIds = nodeIdSetFor(qmlNodeId, add, *_selectionManager);

    if(nodeIds.empty())
        return;

    for(auto nodeId : NodeIdSet(nodeIds))
    {
        auto sources = _graphModel->graph().sourcesOf(nodeId);
        nodeIds.insert(sources.begin(), sources.end());
    }

    _commandManager.execute(ExecutePolicy::Once,
        makeSelectNodesCommand(_selectionManager.get(),
        nodeIds, SelectNodesClear::SelectionAndMask));
}

void Document::selectTargets(QmlNodeId qmlNodeId, bool add)
{
    if(busy())
        return;

    NodeIdSet nodeIds = nodeIdSetFor(qmlNodeId, add, *_selectionManager);

    if(nodeIds.empty())
        return;

    for(auto nodeId : NodeIdSet(nodeIds))
    {
        auto targets = _graphModel->graph().targetsOf(nodeId);
        nodeIds.insert(targets.begin(), targets.end());
    }

    _commandManager.execute(ExecutePolicy::Once,
        makeSelectNodesCommand(_selectionManager.get(),
        nodeIds, SelectNodesClear::SelectionAndMask));
}

void Document::selectNeighbours(QmlNodeId qmlNodeId, bool add)
{
    if(busy())
        return;

    NodeIdSet nodeIds = nodeIdSetFor(qmlNodeId, add, *_selectionManager);

    if(nodeIds.empty())
        return;

    for(auto nodeId : NodeIdSet(nodeIds))
    {
        auto neighbours = _graphModel->graph().neighboursOf(nodeId);
        nodeIds.insert(neighbours.begin(), neighbours.end());
    }

    _commandManager.execute(ExecutePolicy::Once,
        makeSelectNodesCommand(_selectionManager.get(),
        nodeIds, SelectNodesClear::SelectionAndMask));
}

void Document::selectBySharedAttributeValue(const QString& attributeName, QmlNodeId qmlNodeId, bool add)
{
    if(busy() || _selectionManager == nullptr)
        return;

    const NodeIdSet nodeIdSet = nodeIdSetFor(qmlNodeId, add, *_selectionManager);

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

    term = u"^(%1)$"_s.arg(term);

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
    }, {tr("Invert Selection"), tr("Inverting Selection"), tr("Inverted Selection")});
}

void Document::undo()
{
    if(busy())
        return;

    _commandManager.undo();
}

void Document::redo()
{
    if(busy())
        return;

    _commandManager.redo();
}

void Document::rollback()
{
    if(busy())
        return;

    _commandManager.rollback();
}

void Document::deleteNode(QmlNodeId nodeId)
{
    if(busy())
        return;

    _commandManager.execute(ExecutePolicy::Add,
        std::make_unique<DeleteNodesCommand>(_graphModel.get(),
        _selectionManager.get(), NodeIdSet{nodeId}));
}

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

    _graphDisplay->resetView();
}

void Document::switchToOverviewMode()
{
    if(busy())
        return;

    _graphDisplay->switchToOverviewMode();
}

int Document::projection() const
{
    if(_graphDisplay != nullptr)
        return static_cast<int>(_graphDisplay->projection());

    return static_cast<int>(Projection::Perspective);
}

void Document::setProjection(int _projection)
{
    _graphDisplay->setProjection(static_cast<Projection>(_projection));
}

int Document::shading2D() const
{
    if(_graphDisplay != nullptr)
        return static_cast<int>(_graphDisplay->shading2D());

    return static_cast<int>(Shading::Flat);
}

void Document::setShading2D(int _shading2D)
{
    _graphDisplay->setShading2D(static_cast<Shading>(_shading2D));
}

int Document::shading3D() const
{
    if(_graphDisplay != nullptr)
        return static_cast<int>(_graphDisplay->shading3D());

    return static_cast<int>(Shading::Smooth);
}

void Document::setShading3D(int _shading3D)
{
    _graphDisplay->setShading3D(static_cast<Shading>(_shading3D));
}

void Document::gotoPrevComponent()
{
    const auto& componentIds = _graphModel->graph().componentIds();
    auto focusedComponentId = _graphDisplay->focusedComponentId();

    if(busy() || componentIds.empty())
        return;

    if(!focusedComponentId.isNull())
    {
        auto it = std::find(componentIds.begin(), componentIds.end(), focusedComponentId);

        if(it != componentIds.begin())
            --it;
        else
            it = std::prev(componentIds.end());

        _graphDisplay->moveFocusToComponent(*it);
    }
    else
        _graphDisplay->moveFocusToComponent(componentIds.back());
}

void Document::gotoNextComponent()
{
    const auto& componentIds = _graphModel->graph().componentIds();
    auto focusedComponentId = _graphDisplay->focusedComponentId();

    if(busy() || componentIds.empty())
        return;

    if(!focusedComponentId.isNull())
    {
        auto it = std::find(componentIds.begin(), componentIds.end(), focusedComponentId);

        if(std::next(it) != componentIds.end())
            ++it;
        else
            it = componentIds.begin();

        _graphDisplay->moveFocusToComponent(*it);
    }
    else
        _graphDisplay->moveFocusToComponent(componentIds.front());
}

void Document::find(const QString& term, int options, const QStringList& attributeNames, int findSelectStyle)
{
    if(_searchManager == nullptr)
        return;

    _commandManager.executeOnce([this, term, options, attributeNames, findSelectStyle](Command&)
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
    return u::pref(u"misc/focusFoundNodes"_s).toBool() &&
        (!inOverviewMode || u::pref(u"misc/focusFoundComponents"_s).toBool());
}

void Document::selectAndFocusNode(NodeId nodeId)
{
    _commandManager.execute(ExecutePolicy::Once,
        makeSelectNodeCommand(_selectionManager.get(), nodeId),
        [this, nodeId](Command&)
        {
            executeOnMainThread([this, nodeId]
            {
                if(shouldMoveFindFocus(_graphDisplay->inOverviewMode()))
                    _graphDisplay->moveFocusToNode(nodeId);
            });
        });
}

void Document::selectAndFocusNodes(const std::vector<NodeId>& nodeIds)
{
    _commandManager.execute(ExecutePolicy::Once,
        makeSelectNodesCommand(_selectionManager.get(), nodeIds),
        [this, nodeIds](Command&)
        {
            executeOnMainThread([this, nodeIds]
            {
                if(shouldMoveFindFocus(_graphDisplay->inOverviewMode()))
                    _graphDisplay->moveFocusToNodes(nodeIds);
            });
        });
}

void Document::selectAndFocusNodes(const NodeIdSet& nodeIds)
{
    selectAndFocusNodes(u::vectorFrom(nodeIds));
}

void Document::moveFocusToNode(NodeId nodeId)
{
    _graphDisplay->moveFocusToNode(nodeId);
}

void Document::moveFocusToNodes(const std::vector<NodeId>& nodeIds)
{
    _graphDisplay->moveFocusToNodes(nodeIds);
}

void Document::clearSelectedNodes()
{
    selectNone();
}

void Document::selectNodes(const NodeIdSet& nodeIds)
{
    _commandManager.execute(ExecutePolicy::Once,
        makeSelectNodesCommand(_selectionManager.get(), nodeIds));
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
    CrashHandler::instance()->submitMinidump(description);
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

void Document::setSaveRequired()
{
    if(!_loadComplete)
        return;

    _saveRequired = true;
    emit saveRequiredChanged();
}

size_t Document::numNodesSelected() const
{
    if(_selectionManager != nullptr)
        return _selectionManager->numNodesSelected();

    return 0;
}

size_t Document::numHeadNodesSelected() const
{
    size_t numNodes = 0;

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

size_t Document::numInvisibleNodesSelected() const
{
    if(_selectionManager != nullptr)
    {
        auto selectedNodes = _selectionManager->selectedNodes();

        for(auto it = selectedNodes.begin(); it != selectedNodes.end(); /*NO OP*/)
        {
            auto componentIdOfNode = _graphModel->graph().componentIdOfNode(*it);
            if(_graphDisplay->focusedComponentId() == componentIdOfNode)
                it = selectedNodes.erase(it);
            else
                ++it;
        }

        return selectedNodes.size();
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
        const bool nodesWereFound = _foundItValid || searchManager->selectStyle() == FindSelectStyle::All;
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

size_t Document::numNodesFound() const
{
    if(_searchManager != nullptr)
        return _searchManager->foundNodeIds().size();

    return 0;
}

bool Document::nodesMaskActive() const
{
    if(_selectionManager != nullptr)
        return _selectionManager->nodesMaskActive();

    return false;
}

void Document::setFoundIt(std::vector<NodeId>::const_iterator foundIt)
{
    bool changed = !_foundItValid || (_foundIt != foundIt);
    _foundIt = foundIt;

    const bool oldFoundItValid = _foundItValid;
    _foundItValid = (_foundIt != _foundNodeIds.end());

    if(!changed)
        changed = (_foundItValid != oldFoundItValid);

    if(changed)
        emit foundIndexChanged();
}

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
        sourceAttributes.replaceInStrings(QRegularExpression(u"^"_s), u"source."_s);
        targetAttributes.replaceInStrings(QRegularExpression(u"^"_s), u"target."_s);
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

        map.insert(u"elementType"_s, static_cast<int>(elementType));
        map.insert(u"name"_s, transformName);
        map.insert(u"description"_s, transformFactory->description());
        map.insert(u"image"_s, transformFactory->image());
        map.insert(u"requiresCondition"_s, transformFactory->requiresCondition());

        QStringList attributeParameterNames;
        QVariantList attributeParameters;
        for(const auto& attributeParameter : transformFactory->attributeParameters())
        {
            const QVariantMap attributeParameterMap = transformAttributeParameter(transformName, attributeParameter.name());
            attributeParameterNames.append(attributeParameter.name());
            attributeParameters.append(attributeParameterMap);
        }
        map.insert(u"attributeParameterNames"_s, attributeParameterNames);
        map.insert(u"attributeParameters"_s, attributeParameters);

        QStringList parameterNames;
        QVariantMap parameters;
        for(const auto& parameter : transformFactory->parameters())
        {
            const QVariantMap parameterMap = transformParameter(transformName, parameter.name());
            parameterNames.append(parameter.name());
            parameters.insert(parameter.name(), parameterMap);
        }
        map.insert(u"parameterNames"_s, parameterNames);
        map.insert(u"parameters"_s, parameters);

        QVariantMap defaultVisualisations;
        for(const auto& defaultVisualisation : transformFactory->defaultVisualisations())
        {
            QVariantMap defaultVisualisationMap;
            auto attributeName = Attribute::enquoteAttributeName(defaultVisualisation._attributeName);
            defaultVisualisationMap.insert(u"name"_s, attributeName);
            defaultVisualisationMap.insert(u"flags"_s, static_cast<int>(*defaultVisualisation._attributeFlags));
            defaultVisualisationMap.insert(u"elementType"_s, static_cast<int>(defaultVisualisation._elementType));
            defaultVisualisationMap.insert(u"valueType"_s, static_cast<int>(defaultVisualisation._attributeValueType));
            defaultVisualisationMap.insert(u"channelName"_s, defaultVisualisation._channel);
            defaultVisualisations.insert(attributeName, defaultVisualisationMap);
        }
        map.insert(u"defaultVisualisations"_s, defaultVisualisations);
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

    map.insert(u"alertType"_s, static_cast<int>(AlertType::None));
    map.insert(u"alertText"_s, "");

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

    map.insert(u"alertType"_s, static_cast<int>(transformAlert._type));
    map.insert(u"alertText"_s, transformAlert._text);

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
        map.insert(u"name"_s, parameter.name());
        map.insert(u"valueType"_s, static_cast<int>(parameter.type()));

        map.insert(u"hasRange"_s, parameter.hasRange());
        map.insert(u"hasMinimumValue"_s, parameter.hasMin());
        map.insert(u"hasMaximumValue"_s, parameter.hasMax());

        if(parameter.hasMin()) map.insert(u"minimumValue"_s, parameter.min());
        if(parameter.hasMax()) map.insert(u"maximumValue"_s, parameter.max());

        map.insert(u"description"_s, parameter.description());
        map.insert(u"initialValue"_s, parameter.initialValue());

        if(!parameter.validatorRegex().isEmpty())
            map.insert(u"validatorRegex"_s, parameter.validatorRegex());

        // If it's a StringList parameter, select the first one by default
        if(parameter.type() == ValueType::StringList)
            map.insert(u"initialIndex"_s, 0);
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
        map.insert(u"name"_s, parameter.name());
        map.insert(u"elementType"_s, static_cast<int>(parameter.elementType()));
        map.insert(u"valueType"_s, static_cast<int>(parameter.valueType()));
        map.insert(u"description"_s, parameter.description());
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

        map.insert(u"name"_s, u"%1%2"_s
            .arg(prefix, parsedAttributeName._name));

        const bool hasParameter = attribute.hasParameter();
        map.insert(u"hasParameter"_s, hasParameter);

        if(hasParameter)
        {
            if(!parsedAttributeName._parameter.isEmpty())
                GraphModel::calculateAttributeRange(&graphModel()->mutableGraph(), attribute);

            map.insert(u"parameterValue"_s, parsedAttributeName._parameter);
            map.insert(u"isValid"_s, !parsedAttributeName._parameter.isEmpty());
            map.insert(u"validParameterValues"_s, attribute.validParameterValues());
        }
        else
            map.insert(u"isValid"_s, true);

        map.insert(u"flags"_s, static_cast<int>(attribute.flags()));
        map.insert(u"valueType"_s, static_cast<int>(attribute.valueType()));
        map.insert(u"elementType"_s, static_cast<int>(attribute.elementType()));
        map.insert(u"userDefined"_s, attribute.userDefined());
        map.insert(u"metaData"_s, attribute.metaData());
        map.insert(u"editable"_s, attribute.editable());

        map.insert(u"hasRange"_s, attribute.numericRange().hasRange());
        map.insert(u"hasMinimumValue"_s, attribute.numericRange().hasMin());
        map.insert(u"hasMaximumValue"_s, attribute.numericRange().hasMax());

        if(attribute.numericRange().hasMin()) map.insert(u"minimumValue"_s, attribute.numericRange().min());
        if(attribute.numericRange().hasMax()) map.insert(u"maximumValue"_s, attribute.numericRange().max());

        map.insert(u"description"_s, attribute.description());
        map.insert(u"ops"_s, _graphModel->avaliableConditionFnOps(parsedAttributeName._name));

        QStringList sharedValues;
        const auto& sharedValueCounts = attribute.sharedValues();
        sharedValues.reserve(static_cast<int>(sharedValueCounts.size()));
        for(const auto& sharedValueCount : sharedValueCounts)
            sharedValues.append(sharedValueCount._value);

        map.insert(u"sharedValues"_s, sharedValues);
    }
    else
        map.insert(u"isValid"_s, false);

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
        attributeValues.reserve(static_cast<int>(_graphModel->mutableGraph().numNodes()));

        for(auto nodeId : _graphModel->mutableGraph().nodeIds())
            attributeValues.append(attribute->stringValueOf(nodeId));
    }
    else if(attribute->elementType() == ElementType::Edge)
    {
        attributeValues.reserve(static_cast<int>(_graphModel->mutableGraph().numEdges()));

        for(auto edgeId : _graphModel->mutableGraph().edgeIds())
            attributeValues.append(attribute->stringValueOf(edgeId));
    }

    return attributeValues;
}

QStringList Document::addedOrChangedAttributeNamesAtTransformIndexOrLater(int firstIndex) const
{
    return u::toQStringList(_graphModel->addedOrChangedAttributeNamesAtTransformIndexOrLater(firstIndex));
}

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

bool Document::graphTransformsAreValid(const QStringList& transforms) const
{
    if(_graphModel == nullptr)
        return false;

    return std::all_of(transforms.begin(), transforms.end(),
        [this](const auto& transform)
        {
            return _graphModel->graphTransformIsValid(transform);
    });
}

QString Document::displayTextForGraphTransform(const QString& transform) const
{
    return GraphTransformConfigParser::parseForDisplay(transform);
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
static bool transformsDiffer(const QStringList& a, const QStringList& b,
    bool ignoreInertFlags = true)
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

        if(!ai.equals(bi, ignoreInertFlags))
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
        _graphModel.get(), this,
        _graphTransforms, newGraphTransforms));
}

QStringList Document::availableVisualisationChannelNames(int elementType, int valueType) const
{
    return _graphModel != nullptr ? _graphModel->availableVisualisationChannelNames(
        static_cast<ElementType>(elementType), static_cast<ValueType>(valueType)) : QStringList();
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

    map.insert(u"alertType"_s, static_cast<int>(AlertType::None));
    map.insert(u"alertText"_s, "");
    map.insert(u"minimumNumericValue"_s, 0.0);
    map.insert(u"maximumNumericValue"_s, 1.0);
    map.insert(u"mappedMinimumNumericValue"_s, 0.0);
    map.insert(u"mappedMaximumNumericValue"_s, 1.0);
    map.insert(u"hasNumericRange"_s, true);
    map.insert(u"numericValues"_s, numericValues);
    map.insert(u"stringValues"_s, stringValues);
    map.insert(u"numApplications"_s, 1);

    if(_graphModel == nullptr)
        return map;

    const auto& visualisationInfo = _graphModel->visualisationInfoAtIndex(index);

    map.insert(u"minimumNumericValue"_s, visualisationInfo.statistics()._min);
    map.insert(u"maximumNumericValue"_s, visualisationInfo.statistics()._max);
    map.insert(u"mappedMinimumNumericValue"_s, visualisationInfo.mappedMinimum());
    map.insert(u"mappedMaximumNumericValue"_s, visualisationInfo.mappedMaximum());
    map.insert(u"hasNumericRange"_s, visualisationInfo.statistics()._range > 0.0);
    map.insert(u"numApplications"_s, static_cast<int>(visualisationInfo.numApplications()));

    const auto& numericValuesVector = visualisationInfo.statistics()._values;
    numericValues.reserve(static_cast<int>(numericValuesVector.size()));
    for(auto value : numericValuesVector)
        numericValues.append(value);

    map.insert(u"numericValues"_s, numericValues);

    const auto& stringValuesVector = visualisationInfo.stringValues();
    stringValues.reserve(static_cast<int>(stringValuesVector.size()));
    for(const auto& stringValue : stringValuesVector)
        stringValues.append(stringValue);

    map.insert(u"stringValues"_s, stringValues);

    auto alerts = visualisationInfo.alerts();

    if(alerts.empty())
        return map;

    std::sort(alerts.begin(), alerts.end(),
    [](auto& a, auto& b)
    {
        return a._type > b._type;
    });

    auto& alert = alerts.at(0);

    map.insert(u"alertType"_s, static_cast<int>(alert._type));
    map.insert(u"alertText"_s, alert._text);

    return map;
}

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

bool Document::visualisationsAreValid(const QStringList& visualisations) const
{
    if(_graphModel == nullptr)
        return false;

    return std::all_of(visualisations.begin(), visualisations.end(),
        [this](const auto& visualisation)
        {
            return _graphModel->visualisationIsValid(visualisation);
    });
}

QString Document::displayTextForVisualisation(const QString& visualisation) const
{
    return VisualisationConfigParser::parseForDisplay(visualisation);
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

        if(!ai.equals(bi))
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

void Document::apply(const QStringList& graphTransforms, const QStringList& visualisations, bool replaceLatestCommand)
{
    if(_graphModel == nullptr)
        return;

    ICommandPtrsVector commands;

    const bool transformsValid = graphTransformsAreValid(graphTransforms);
    int newGraphTransformIndex = -1;

    if(transformsValid && transformsDiffer(_graphTransforms, graphTransforms, true))
    {
        commands.emplace_back(std::make_unique<ApplyTransformsCommand>(
            _graphModel.get(), this,
            _graphTransforms, graphTransforms));

        // This is necessary for the visualisation patching in ApplyVisualisationsCommand,
        // see the comments in there for more information
        auto difference = u::setDifference(graphTransforms, _graphTransforms);
        if(difference.size() == 1)
            newGraphTransformIndex = u::indexOf(graphTransforms, difference.at(0));
    }
    else if(transformsValid && transformsDiffer(_graphTransforms, graphTransforms, false))
    {
        auto previousGraphTransforms = _graphTransforms;

        commands.emplace_back(std::make_unique<Command>(
            Command::CommandDescription
            {
                u"Apply Transform Flags"_s,
                u"Applying Transform Flags"_s
            },
            [this, graphTransforms](Command&)           { setTransforms(graphTransforms); },
            [this, previousGraphTransforms](Command&)   { setTransforms(previousGraphTransforms); }));
    }
    else
        setTransforms(_graphTransforms);

    _graphModel->clearVisualisationInfos();

    if(visualisationsDiffer(_visualisations, visualisations))
    {
        commands.emplace_back(std::make_unique<ApplyVisualisationsCommand>(
            _graphModel.get(), this,
            _visualisations, visualisations, newGraphTransformIndex));
    }
    else
        setVisualisations(_visualisations);

    const ExecutePolicy policy = replaceLatestCommand ?
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

void Document::update(const QStringList& newGraphTransforms, const QStringList& newVisualisations, bool replaceLatestCommand)
{
    auto graphTransforms = graphTransformConfigurationsFromUI();

    for(const auto& newGraphTransform : std::as_const(newGraphTransforms))
    {
        if(!transformIsPinned(newGraphTransform))
        {
            // Insert before any existing pinned transforms
            int index = 0;
            while(index < graphTransforms.size() && !transformIsPinned(graphTransforms.at(index)))
                index++;

            graphTransforms.insert(index, newGraphTransform);
        }
        else
            graphTransforms.append(newGraphTransform);
    }

    auto visualisations = _visualisationsFromUI;
    visualisations.append(newVisualisations);

    apply(graphTransforms, visualisations, replaceLatestCommand);
}

QVariantMap Document::layoutSetting(const QString& name) const
{
    QVariantMap map;

    const auto* setting = _layoutThread->setting(name);
    if(setting != nullptr)
    {
        map.insert(u"name"_s, setting->name());
        map.insert(u"displayName"_s, setting->displayName());
        map.insert(u"description"_s, setting->description());
        map.insert(u"value"_s, setting->value());
        map.insert(u"normalisedValue"_s, setting->normalisedValue());
        map.insert(u"minimumValue"_s, setting->minimumValue());
        map.insert(u"maximumValue"_s, setting->maximumValue());
        map.insert(u"defaultValue"_s, setting->defaultValue());
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
        return u::pref(u"visuals/defaultNormalNodeSize"_s).toFloat();

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
    auto defaultNormalNodeSize = u::pref(u"visuals/defaultNormalNodeSize"_s).toFloat();
    setNodeSize(defaultNormalNodeSize);
}

float Document::edgeSize() const
{
    if(_graphModel == nullptr)
        return u::pref(u"visuals/defaultNormalEdgeSize"_s).toFloat();

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
    auto defaultNormalEdgeSize = u::pref(u"visuals/defaultNormalEdgeSize"_s).toFloat();
    setEdgeSize(defaultNormalEdgeSize);
}

float Document::textSize() const
{
    if(_graphModel == nullptr)
        return 1.0f;

    return _graphModel->textSize();
}

void Document::setTextSize(float textSize)
{
    if(_graphModel == nullptr)
        return;

    if(textSize != _graphModel->textSize())
    {
        _graphModel->setTextSize(textSize);
        emit textSizeChanged();
        setSaveRequired();
    }
}

void Document::resetTextSize()
{
    setTextSize(LimitConstants::defaultTextSize());
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

void Document::writeTableModelToFile(QAbstractItemModel* model, const QUrl& fileUrl,
    const QString& extension, const QStringList& columnHeaders)
{
    const QString localFileName = fileUrl.toLocalFile();
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
            static const QRegularExpression re(u"[\",]"_s);
            if(string.contains(re))
            {
                QString escaped = string;

                // Encode " as ""
                escaped.replace(u"\""_s, u"\"\""_s);

                return u"\"%1\""_s.arg(escaped);
            }

            return string;
        };

        auto tsvEscapedString = [](const QString& string)
        {
            QString escaped = string;

            // "The IANA standard for TSV achieves simplicity
            // by simply disallowing tabs within fields."
            return escaped.replace(u"\t"_s, u""_s);
        };

        std::function<QString(const QString&)> escapedString = csvEscapedString;
        QString separator = u","_s;

        if(extension == u"tsv"_s)
        {
            escapedString = tsvEscapedString;
            separator = u"\t"_s;
        }

        QTextStream stream(&file);
        QString rowString;

        if(!columnHeaders.isEmpty())
        {
            for(const auto& columnName : columnHeaders)
            {
                if(!rowString.isEmpty())
                    rowString.append(separator);

                rowString.append(escapedString(columnName));
            }

            stream << rowString << "\r\n";
        }

        if(model != nullptr)
        {
            for(int row = 0; row < model->rowCount(); row++)
            {
                rowString.clear();
                for(int column = 0; column < model->columnCount(); column++)
                {
                    if(!rowString.isEmpty())
                        rowString.append(separator);

                    auto value = model->data(model->index(row, column));

                    auto valueString = value.toString();

                    if(value.typeId() == QMetaType::QString)
                        rowString.append(escapedString(valueString));
                    else
                        rowString.append(valueString);
                }

                stream << rowString << "\r\n";
            }
        }
    }, {tr("Export Table"), tr("Exporting Table"), tr("Exported Table")});
}

void Document::copyTableModelColumnToClipboard(QAbstractItemModel* model, int column, const QVector<int>& rows)
{
    if(column < 0 || column >= model->columnCount())
    {
        qDebug() << "Document::copyTableModelColumnToClipboard: requested column exceeds column count";
        return;
    }

    QString text;

    for(auto row : rows)
    {
        auto value = model->data(model->index(row, column));
        text.append(u"%1\n"_s.arg(value.toString()));
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
    if(!attributeExists(selectedAttributeA) || !attributeExists(selectedAttributeB))
    {
        qDebug() << "ERROR: performEnrichment attribute doesn't exist.";
        return;
    }

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
    const QString localFileName = fileUrl.toLocalFile();
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
            const auto& nodePositions = _graphModel->nodePositions();
            auto v = nodePositions.at(nodeId);

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
    }, {tr("Export Node Positions"), tr("Exporting Node Positions"), tr("Exported Node Positions")});
}

void Document::loadNodePositionsFromFile(const QUrl& fileUrl)
{
    // If we're loading a static layout, ensure any dynamic layout is
    // stopped or the loaded layout will just get immediately replaced
    setUserLayoutPaused(true);

    const QString localFileName = fileUrl.toLocalFile();

    commandManager()->executeOnce(
    [this, localFileName](Command& command)
    {
        QFile file(localFileName);

        if(!file.open(QIODevice::ReadOnly))
            return false;

        auto totalBytes = file.size();

        if(totalBytes == 0)
            return false;

        decltype(totalBytes) bytesRead = 0;
        QDataStream input(&file);
        QByteArray byteArray;

        do
        {
            const int ChunkSize = 2 << 16;
            std::vector<unsigned char> buffer(ChunkSize);

            auto numBytes = input.readRawData(reinterpret_cast<char*>(buffer.data()), ChunkSize);
            byteArray.append(reinterpret_cast<char*>(buffer.data()), static_cast<qsizetype>(numBytes));

            bytesRead += numBytes;

            command.setProgress(static_cast<int>((bytesRead * 100u) / totalBytes));
        } while(!input.atEnd());

        command.setProgress(-1);
        auto jsonArray = parseJsonFrom(byteArray);

        if(jsonArray.is_null() || !jsonArray.is_array())
            return false;

        auto allObjects = std::all_of(jsonArray.begin(), jsonArray.end(),
            [](const auto& i) { return i.is_object(); });

        if(!allObjects)
            return false;

        ExactNodePositions nodePositions(_graphModel->mutableGraph());

        uint64_t i = 0;

        for(const auto& jsonNode : jsonArray)
        {
            if(!u::contains(jsonNode, "id") || !u::contains(jsonNode, "position"))
                return false;

            const NodeId nodeId = jsonNode["id"].get<int>();

            if(!_graphModel->mutableGraph().containsNodeId(nodeId))
                return false;

            auto positionArray = jsonNode["position"];

            if(positionArray.is_null() || !positionArray.is_array() || positionArray.size() != 3)
                return false;

            const QVector3D position(positionArray.at(0), positionArray.at(1), positionArray.at(2));
            nodePositions.set(nodeId, position);

            command.setProgress(static_cast<int>((i++ * 100u) /
                static_cast<uint64_t>(jsonArray.size())));
        }

        _layoutThread->setNodePositions(nodePositions);

        return true;
    }, {tr("Import Node Positions"), tr("Importing Node Positions"), tr("Imported Node Positions")});
}

void Document::cloneAttribute(const QString& sourceAttributeName, const QString& newAttributeName)
{
    if(busy())
        return;

    _commandManager.execute(ExecutePolicy::Add,
        std::make_unique<CloneAttributeCommand>(_graphModel.get(), sourceAttributeName, newAttributeName));
}

void Document::editAttribute(const QString& attributeName, const AttributeEdits& edits,
    ValueType newType, const QString& newDescription)
{
    if(busy())
        return;

    _commandManager.execute(ExecutePolicy::Add,
        std::make_unique<EditAttributeCommand>(_graphModel.get(),
        Attribute::parseAttributeName(attributeName)._name,
        edits, newType, newDescription));
}

void Document::removeAttributes(const QStringList& attributeNames)
{
    if(busy())
        return;

    _commandManager.execute(ExecutePolicy::Add,
        std::make_unique<RemoveAttributesCommand>(_graphModel.get(), attributeNames));
}

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

    text += u"Mutable Graph Nodes: %1 Edges: %2\n"_s
        .arg(_graphModel->mutableGraph().numNodes())
        .arg(_graphModel->mutableGraph().numEdges());

    text += u"Transformed Graph Nodes: %1 Edges: %2 Components: %3"_s
        .arg(_graphModel->graph().numNodes())
        .arg(_graphModel->graph().numEdges())
        .arg(_graphModel->graph().numComponents());

    return text;
}

QString Document::commandStackSummary() const
{
    return _commandManager.commandStackSummary();
}

void Document::startTestCommand()
{
    class TestCommand : public ICommand
    {
    public:
        bool execute() override
        {
            while(!cancelled())
            {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(1000ms);
            }

            return true;
        }

        QString description() const override { return QObject::tr("Test Command"); }
        bool cancellable() const override { return true; }
    };

    _commandManager.execute(ExecutePolicy::Once, std::make_unique<TestCommand>());
}
