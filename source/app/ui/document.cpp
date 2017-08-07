#include "document.h"

#include "application.h"

#include "shared/plugins/iplugin.h"
#include "shared/utils/preferences.h"
#include "shared/utils/flags.h"
#include "shared/utils/utils.h"

#include "graph/graphmodel.h"
#include "loading/parserthread.h"

#include "layout/forcedirectedlayout.h"
#include "layout/layout.h"
#include "layout/collision.h"

#include "commands/deleteselectednodescommand.h"
#include "commands/applytransformscommand.h"
#include "commands/applyvisualisationscommand.h"
#include "commands/selectnodescommand.h"

#include "transform/graphtransformconfigparser.h"
#include "ui/visualisations/visualisationconfigparser.h"

#include "searchmanager.h"
#include "selectionmanager.h"
#include "graphquickitem.h"

QColor Document::contrastingColorForBackground()
{
    auto backColor = u::pref("visuals/backgroundColor").value<QColor>();
    return u::contrastingColor(backColor);
}

Document::Document(QObject* parent) :
    QObject(parent),
    _graphChanging(false),
    _layoutRequired(true),
    _graphTransformsModel(this)
{}

Document::~Document()
{
    // Execute anything pending (primarily to avoid deadlock)
    executeDeferred();

    // This must be called from the main thread before deletion
    if(_gpuComputeThread != nullptr)
        _gpuComputeThread->destroySurface();
}

bool Document::commandInProgress() const
{
    return !_loadComplete || _commandManager.busy();
}

bool Document::idle() const
{
    return !commandInProgress() && !_graphChanging && !_graphQuickItem->interacting();
}

void Document::maybeEmitIdleChanged()
{
    if(idle() != _previousIdle)
    {
        _previousIdle = idle();
        emit idleChanged();
    }
}

bool Document::canDelete() const
{
    if(_selectionManager != nullptr)
        return idle() && !_selectionManager->selectedNodes().empty() && _graphModel->editable();

    return false;
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
            return QString(tr("Loading %1 (%2)").arg(_title).arg(phase));

        return QString(tr("Loading %1").arg(_title));
    }

    if(!phase.isEmpty())
        return QString(tr("%1 (%2)")).arg(_commandManager.commandVerb()).arg(phase);

    return _commandManager.commandVerb();
}

bool Document::commandIsCancellable() const
{
    return _commandManager.commandIsCancellable();
}

void Document::updateLayoutState()
{
    if(idle() && !_userLayoutPaused && _layoutRequired)
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

void Document::toggleLayout()
{
    if(!idle())
        return;

    _userLayoutPaused = !_userLayoutPaused;
    _layoutRequired = true;
    emit layoutPauseStateChanged();

    updateLayoutState();
}

bool Document::canUndo() const
{
    return idle() && _commandManager.canUndo();
}

QString Document::nextUndoAction() const
{
    return _commandManager.nextUndoAction();
}

bool Document::canRedo() const
{
    return idle() && _commandManager.canRedo();
}

QString Document::nextRedoAction() const
{
    return _commandManager.nextRedoAction();
}

bool Document::canResetView() const
{
    return idle() && !_graphQuickItem->viewIsReset();
}

bool Document::canEnterOverviewMode() const
{
    return idle() && _graphQuickItem->canEnterOverviewMode();
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
    // This stores the current active configuration...
    _graphTransforms = transforms;

    // ...while the model has the state of the UI
    _graphTransformsModel.clear();
    for(const auto& transform : transforms)
        _graphTransformsModel.append(transform);
}

void Document::setVisualisations(const QStringList& visualisations)
{
    _visualisations = visualisations;

    _visualisationsModel.clear();
    for(const auto& visualisation : visualisations)
        _visualisationsModel.append(visualisation);
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

static bool transformIsPinned(const QString& transform)
{
    GraphTransformConfigParser p;

    if(!p.parse(transform)) return false;
    return p.result().isFlagSet("pinned");
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
        else if(!aPinned && bPinned)
            return true;

        return false;
    });

    return transforms;
}

QStringList Document::graphTransformConfigurationsFromUI() const
{
    QStringList transforms;

    for(const auto& variant : _graphTransformsModel.list())
        transforms.append(variant.toString());

    return sortedTransforms(transforms);
}

QStringList Document::visualisationsFromUI() const
{
    QStringList visualisations;

    for(const auto& variant : _visualisationsModel.list())
        visualisations.append(variant.toString());

    return visualisations;
}

void Document::initialiseLayoutSettingsModel()
{
    _layoutSettingsModel.clear();
    for(const auto& setting : _layoutThread->settings())
        _layoutSettingsModel.append(setting.name());
}

bool Document::openFile(const QUrl& fileUrl, const QString& fileType, const QString& pluginName, const QVariantMap& parameters)
{
    auto* plugin = _application->pluginForName(pluginName);
    if(plugin == nullptr)
        return false;

    setTitle(fileUrl.fileName());
    emit commandInProgressChanged();
    emit idleChanged();
    emit commandVerbChanged(); // Show Loading message

    _graphModel = std::make_unique<GraphModel>(fileUrl.fileName(), plugin);

    _gpuComputeThread = std::make_unique<GPUComputeThread>();
    _graphFileParserThread = std::make_unique<ParserThread>(_graphModel->mutableGraph(), fileUrl);

    _selectionManager = std::make_unique<SelectionManager>(*_graphModel);
    _searchManager = std::make_unique<SearchManager>(*_graphModel);

    _pluginInstance = plugin->createInstance();
    _pluginInstance->initialise(_graphModel.get(), _selectionManager.get(), &_commandManager, _graphFileParserThread.get());

    connect(S(Preferences), &Preferences::preferenceChanged, this, &Document::onPreferenceChanged, Qt::DirectConnection);
    connect(&_graphModel->graph(), &Graph::graphChanged, [this]
    {
        _searchManager->refresh();
        _graphModel->updateVisuals(_selectionManager.get(), _searchManager.get());
    });
    connect(_searchManager.get(), &SearchManager::foundNodeIdsChanged, this, &Document::numNodesFoundChanged);

    connect(&_graphModel->mutableGraph(), &Graph::phaseChanged, this, &Document::commandVerbChanged);

    for(const auto& name : parameters.keys())
        _pluginInstance->applyParameter(name, parameters.value(name).toString());

    emit pluginInstanceChanged();

    auto parser = _pluginInstance->parserForUrlTypeName(fileType);
    if(parser == nullptr)
    {
        qDebug() << "Plugin does not provide parser";
        return false;
    }

    connect(_graphFileParserThread.get(), &ParserThread::progress, this, &Document::onLoadProgress);
    connect(_graphFileParserThread.get(), &ParserThread::success, [this]
    {
        _graphModel->buildTransforms(sortedTransforms(_pluginInstance->defaultTransforms()));
        _graphModel->buildVisualisations(_pluginInstance->defaultVisualisations());
    });
    connect(_graphFileParserThread.get(), &ParserThread::complete, this, &Document::onLoadComplete);
    _graphFileParserThread->start(std::move(parser));

    return true;
}

void Document::onPreferenceChanged(const QString& key, const QVariant&)
{
    if(key == "visuals/backgroundColor")
        emit contrastingColorChanged();
}

void Document::onLoadProgress(int percentage)
{
    _loadProgress = percentage;
    emit commandProgressChanged();
    emit commandVerbChanged();
}

void Document::onLoadComplete(bool success)
{
    Q_ASSERT(success);
    Q_UNUSED(success);

    _loadComplete = true;
    emit commandInProgressChanged();
    emit idleChanged();
    emit commandVerbChanged(); // Stop showing loading message

    // This causes the plugin UI to be loaded
    emit pluginQmlPathChanged();

    setTransforms(sortedTransforms(_pluginInstance->defaultTransforms()));
    setVisualisations(_pluginInstance->defaultVisualisations());

    _layoutThread = std::make_unique<LayoutThread>(*_graphModel, std::make_unique<ForceDirectedLayoutFactory>(_graphModel.get()));
    connect(_layoutThread.get(), &LayoutThread::pausedChanged, this, &Document::layoutPauseStateChanged);
    connect(_layoutThread.get(), &LayoutThread::settingChanged, [this] { _layoutRequired = true; });
    connect(_layoutThread.get(), &LayoutThread::settingChanged, this, &Document::updateLayoutState);
    _layoutThread->addAllComponents();
    initialiseLayoutSettingsModel();

    _graphQuickItem->initialise(_graphModel.get(), &_commandManager, _selectionManager.get(), _gpuComputeThread.get());

    connect(_graphQuickItem, &GraphQuickItem::interactingChanged, this, &Document::maybeEmitIdleChanged, Qt::DirectConnection);
    connect(_graphQuickItem, &GraphQuickItem::viewIsResetChanged, this, &Document::canResetViewChanged);
    connect(_graphQuickItem, &GraphQuickItem::canEnterOverviewModeChanged, this, &Document::canEnterOverviewModeChanged);
    connect(_graphQuickItem, &GraphQuickItem::fpsChanged, this, &Document::fpsChanged);

    connect(&_commandManager, &CommandManager::busyChanged, this, &Document::maybeEmitIdleChanged, Qt::DirectConnection);

    connect(this, &Document::idleChanged, this, &Document::updateLayoutState, Qt::DirectConnection);

    connect(this, &Document::idleChanged, this, &Document::canDeleteChanged);
    connect(this, &Document::idleChanged, this, &Document::canUndoChanged);
    connect(this, &Document::idleChanged, this, &Document::canRedoChanged);
    connect(this, &Document::idleChanged, this, &Document::canEnterOverviewModeChanged);
    connect(this, &Document::idleChanged, this, &Document::canResetViewChanged);

    connect(this, &Document::taskAddedToExecutor, this, &Document::executeDeferred);

    connect(&_commandManager, &CommandManager::commandWillExecute, _graphQuickItem, &GraphQuickItem::commandWillExecute);
    connect(&_commandManager, &CommandManager::commandWillExecute, this, &Document::commandInProgressChanged);

    connect(&_commandManager, &CommandManager::commandProgressChanged, this, &Document::commandProgressChanged);
    connect(&_commandManager, &CommandManager::commandVerbChanged, this, &Document::commandVerbChanged);
    connect(&_commandManager, &CommandManager::commandIsCancellableChanged, this, &Document::commandIsCancellableChanged);

    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::canUndoChanged);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::nextUndoActionChanged);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::canRedoChanged);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::nextRedoActionChanged);
    connect(&_commandManager, &CommandManager::commandCompleted, [this](bool, QString, QString pastParticiple)
    {
        setStatus(pastParticiple);
    });

    connect(&_commandManager, &CommandManager::commandStackCleared, this, &Document::canUndoChanged);
    connect(&_commandManager, &CommandManager::commandStackCleared, this, &Document::nextUndoActionChanged);
    connect(&_commandManager, &CommandManager::commandStackCleared, this, &Document::canRedoChanged);
    connect(&_commandManager, &CommandManager::commandStackCleared, this, &Document::nextRedoActionChanged);

    connect(&_commandManager, &CommandManager::commandCompleted, _graphQuickItem, &GraphQuickItem::commandCompleted);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::commandInProgressChanged);

    connect(_selectionManager.get(), &SelectionManager::selectionChanged, this, &Document::canDeleteChanged);
    connect(_selectionManager.get(), &SelectionManager::selectionChanged, this, &Document::onSelectionChanged);
    connect(_selectionManager.get(), &SelectionManager::selectionChanged,
            _graphModel.get(), &GraphModel::onSelectionChanged);

    connect(_searchManager.get(), &SearchManager::foundNodeIdsChanged, this, &Document::onFoundNodeIdsChanged);
    connect(_searchManager.get(), &SearchManager::foundNodeIdsChanged,
            _graphModel.get(), &GraphModel::onFoundNodeIdsChanged, Qt::DirectConnection);

    connect(_layoutThread.get(), &LayoutThread::executed, _graphQuickItem, &GraphQuickItem::onLayoutChanged);

    connect(&_graphModel->graph(), &Graph::graphWillChange, [this]
    {
        _graphChanging = true;
        maybeEmitIdleChanged();
    });

    connect(&_graphModel->graph(), &Graph::graphChanged, [this]
    (const Graph*, bool changeOccurred)
    {
        _graphChanging = false;
        _layoutRequired = changeOccurred || _layoutRequired;
        maybeEmitIdleChanged();

        // If the graph has changed outside of a Command, then our new state is
        // inconsistent wrt the CommandManager, so throw away our undo history
        if(!commandInProgress())
            _commandManager.clearCommandStack();
    });

    connect(&_graphModel->graph(), &Graph::graphChanged, this, &Document::onGraphChanged);

    connect(&_graphModel->graph(), &Graph::graphChanged, &_commandManager,
            &CommandManager::onGraphChanged, Qt::DirectConnection);

    connect(&_graphModel->mutableGraph(), &Graph::graphChanged, this, &Document::onMutableGraphChanged);

    _graphModel->enableVisualUpdates();

    setStatus(QString(tr("Loaded %1 (%2 nodes, %3 edges, %4 components)")).arg(
                _graphModel->name()).arg(
                _graphModel->graph().numNodes()).arg(
                _graphModel->graph().numEdges()).arg(
                _graphModel->graph().numComponents()));
}

void Document::selectAll()
{
    if(!idle())
        return;

    if(_selectionManager)
    {
        _commandManager.executeOnce({tr("Select All"), tr("Selecting All")},
            [this](Command& command)
            {
                bool nodesSelected = _selectionManager->selectAllNodes();
                command.setPastParticiple(_selectionManager->numNodesSelectedAsString());
                return nodesSelected;
            });
    }
}

void Document::selectNone()
{
    if(!idle())
        return;

    if(_selectionManager && !_selectionManager->selectedNodes().empty())
    {
        _commandManager.executeOnce({tr("Select None"), tr("Selecting None")},
            [this](Command&) { return _selectionManager->clearNodeSelection(); });
    }
}

void Document::invertSelection()
{
    if(!idle())
        return;

    if(_selectionManager)
    {
        _commandManager.executeOnce({tr("Invert Selection"), tr("Inverting Selection")},
            [this](Command& command)
            {
                _selectionManager->invertNodeSelection();
                command.setPastParticiple(_selectionManager->numNodesSelectedAsString());
            });
    }
}

void Document::undo()
{
    if(!idle())
        return;

    _commandManager.undo();
}

void Document::redo()
{
    if(!idle())
        return;

    _commandManager.redo();
}

void Document::deleteSelectedNodes()
{
    if(!idle())
        return;

    if(_selectionManager->selectedNodes().empty())
        return;

    _commandManager.execute(std::make_unique<DeleteSelectedNodesCommand>(_graphModel.get(), _selectionManager.get()));
}

void Document::resetView()
{
    if(!idle())
        return;

    _graphQuickItem->resetView();
}

void Document::switchToOverviewMode(bool doTransition)
{
    if(!idle())
        return;

    _graphQuickItem->switchToOverviewMode(doTransition);
}

static auto componentIdIterator(ComponentId componentId, const std::vector<ComponentId>& componentIds)
{
    Q_ASSERT(!componentId.isNull());
    Q_ASSERT(std::is_sorted(componentIds.begin(), componentIds.end()));
    return std::lower_bound(componentIds.begin(), componentIds.end(), componentId);
}

void Document::gotoPrevComponent()
{
    const auto& componentIds = _graphModel->graph().componentIds();
    auto focusedComponentId = _graphQuickItem->focusedComponentId();

    if(!idle() || componentIds.empty())
        return;

    if(!focusedComponentId.isNull())
    {
        auto it = componentIdIterator(focusedComponentId, componentIds);

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

    if(!idle() || componentIds.empty())
        return;

    if(!focusedComponentId.isNull())
    {
        auto it = componentIdIterator(focusedComponentId, componentIds);

        if(std::next(it) != componentIds.end())
            ++it;
        else
            it = componentIds.begin();

        _graphQuickItem->moveFocusToComponent(*it);
    }
    else
        _graphQuickItem->moveFocusToComponent(componentIds.front());
}

void Document::find(const QString& regex)
{
    _commandManager.executeOnce([this, regex](Command&)
    {
        int previousNumNodesFound = numNodesFound();

        _searchManager->findNodes(regex);

        if(previousNumNodesFound != numNodesFound())
            emit numNodesFoundChanged();
    });
}

static bool shouldMoveFindFocus(bool inOverviewMode)
{
    return u::pref("misc/focusFoundNodes").toBool() &&
        ((inOverviewMode && u::pref("misc/focusFoundComponents").toBool()) || !inOverviewMode);
}

void Document::selectFoundNode(NodeId newFound)
{
    _commandManager.executeOnce(makeSelectNodeCommand(_selectionManager.get(), newFound));

    if(shouldMoveFindFocus(_graphQuickItem->inOverviewMode()))
        _graphQuickItem->moveFocusToNode(newFound);
}

void Document::selectFirstFound()
{
    selectFoundNode(*_foundNodeIds.begin());
}

void Document::selectNextFound()
{
    selectFoundNode(incrementFoundIt());
}

void Document::selectPrevFound()
{
    selectFoundNode(decrementFoundIt());
}

void Document::selectAllFound()
{
    _commandManager.executeOnce(makeSelectNodesCommand(_selectionManager.get(), _searchManager->foundNodeIds()));
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
        _foundItValid = false;
        emit foundIndexChanged();
    }
}

void Document::onSelectionChanged(const SelectionManager*)
{
    updateFoundIndex(false);
}

void Document::onFoundNodeIdsChanged(const SearchManager* searchManager)
{
    _foundNodeIds.clear();

    if(searchManager->foundNodeIds().empty())
    {
        if(_foundItValid)
            _selectionManager->clearNodeSelection();

        _foundItValid = false;
        emit foundIndexChanged();

        return;
    }

    for(auto nodeId : searchManager->foundNodeIds())
        _foundNodeIds.emplace_back(nodeId);

    std::sort(_foundNodeIds.begin(), _foundNodeIds.end(), [this](auto a, auto b)
    {
        auto componentIdA = _graphModel->graph().componentIdOfNode(a);
        auto componentIdB = _graphModel->graph().componentIdOfNode(b);

        if(componentIdA == componentIdB)
            return a < b;
        else
            return componentIdA < componentIdB;
    });

    // _foundNodeIds is potentially in a different memory location,
    // so the iterator is now invalid
    _foundItValid = false;

    if(_selectionManager->selectedNodes().empty())
        selectFirstFound();
    else
        updateFoundIndex(true);
}

void Document::onGraphChanged(const Graph*, bool)
{
    // If the graph changes then so do our visualisations
    _graphModel->buildVisualisations(_visualisations);
    setVisualisations(_visualisations);
}

void Document::onMutableGraphChanged()
{
    // This is only called in order to force the UI to refresh the transform
    // controls, in case the attribute ranges have changed
    setTransforms(_graphTransforms);
}

void Document::executeDeferred()
{
    _deferredExecutor.execute();
    _executed.notify();
}

int Document::foundIndex() const
{
    if(!_foundNodeIds.empty() && _foundItValid)
        return static_cast<int>(std::distance(_foundNodeIds.begin(), _foundIt));

    return -1;
}

int Document::numNodesFound() const
{
    if(_searchManager != nullptr)
        return static_cast<int>(_searchManager->foundNodeIds().size());

    return 0;
}

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

NodeId Document::incrementFoundIt()
{
    NodeId newFound;

    if(_foundItValid && std::next(_foundIt) != _foundNodeIds.end())
        newFound = *(_foundIt + 1);
    else
        newFound = *_foundNodeIds.begin();

    return newFound;
}

NodeId Document::decrementFoundIt()
{
    NodeId newFound;

    if(_foundItValid && _foundIt != _foundNodeIds.begin())
        newFound = *(_foundIt - 1);
    else
        newFound = *std::prev(_foundNodeIds.end());

    return newFound;
}

void Document::executeOnMainThread(DeferredExecutor::TaskFn task,
                                   const QString& description)
{
    _deferredExecutor.enqueue(std::move(task), description);
    emit taskAddedToExecutor();
}

void Document::executeOnMainThreadAndWait(DeferredExecutor::TaskFn task,
                                          const QString& description)
{
    executeOnMainThread(std::move(task), description);
    _executed.wait();
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

AvailableAttributesModel* Document::availableAttributes(int elementTypes, int valueTypes) const
{
    if(_graphModel != nullptr)
    {
        // The caller takes ownership and is responsible for deleting the model
        return new AvailableAttributesModel(*_graphModel, nullptr,
                                            static_cast<ElementType>(elementTypes),
                                            static_cast<ValueType>(valueTypes));
    }

    return nullptr;
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

        map.insert("elementType", static_cast<int>(elementType));
        map.insert("description", transformFactory->description());
        map.insert("requiresCondition", transformFactory->requiresCondition());

        QVariantMap parameters;
        for(const auto& parameter : transformFactory->parameters())
        {
            QVariantMap parameterMap = transformParameter(transformName, parameter.first);
            parameters.insert(parameter.first, parameterMap);
        }
        map.insert("parameters", parameters);

        QVariantMap declaredAttributes;
        for(const auto& declaredAttribute : transformFactory->declaredAttributes())
        {
            QVariantMap declaredAttributeMap;
            declaredAttributeMap.insert("valueType", static_cast<int>(declaredAttribute.second._valueType));
            declaredAttributeMap.insert("defaultVisualisation", declaredAttribute.second._defaultVisualisation);
            declaredAttributes.insert(declaredAttribute.first, declaredAttributeMap);
        }
        map.insert("declaredAttributes", declaredAttributes);
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

    map.insert("alertType", static_cast<int>(AlertType::None));
    map.insert("alertText", "");

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

    map.insert("alertType", static_cast<int>(transformAlert._type));
    map.insert("alertText", transformAlert._text);

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

    if(u::contains(transformFactory->parameters(), parameterName))
    {
        auto parameter = transformFactory->parameters().at(parameterName);
        map.insert("valueType", static_cast<int>(parameter.type()));

        map.insert("hasRange", parameter.hasRange());
        map.insert("hasMinimumValue", parameter.hasMin());
        map.insert("hasMaximumValue", parameter.hasMax());

        if(parameter.hasMin()) map.insert("minimumValue", parameter.min());
        if(parameter.hasMax()) map.insert("maximumValue", parameter.max());

        map.insert("description", parameter.description());
        map.insert("initialValue", parameter.initialValue());
    }

    return map;
}

QVariantMap Document::attribute(const QString& attributeName) const
{
    QVariantMap map;

    if(_graphModel == nullptr)
        return map;

    auto parsedAttributeName = Attribute::parseAttributeName(attributeName);
    if(u::contains(_graphModel->availableAttributes(), parsedAttributeName._name))
    {
        const auto& attribute = _graphModel->attributeValueByName(parsedAttributeName._name);
        map.insert("valueType", static_cast<int>(attribute.valueType()));
        map.insert("elementType", static_cast<int>(attribute.elementType()));

        map.insert("hasRange", attribute.numericRange().hasRange());
        map.insert("hasMinimumValue", attribute.numericRange().hasMin());
        map.insert("hasMaximumValue", attribute.numericRange().hasMax());

        if(attribute.numericRange().hasMin()) map.insert("minimumValue", attribute.numericRange().min());
        if(attribute.numericRange().hasMax()) map.insert("maximumValue", attribute.numericRange().max());

        map.insert("description", attribute.description());
        auto valueType = Flags<ValueType>(attribute.valueType());

        // For similarity purposes, treat Int and Float as the same
        if(valueType.anyOf(ValueType::Int, ValueType::Float))
            valueType.set(ValueType::Int, ValueType::Float);

        map.insert("similar", _graphModel->availableAttributes(attribute.elementType(), *valueType));
        map.insert("ops", _graphModel->avaliableConditionFnOps(parsedAttributeName._name));
    }

    return map;
}

QVariantMap Document::findTransformParameter(const QString& transformName, const QString& parameterName) const
{
    if(_graphModel == nullptr)
        return {};

    if(_graphModel->transformFactory(transformName) == nullptr)
    {
        // Unrecognised transform
        return {};
    }

    auto attributeObject = attribute(parameterName);
    if(!attributeObject.isEmpty())
    {
        // It's an Attribute
        return attributeObject;
    }

    // It's a with ... parameter
    return transformParameter(transformName, parameterName);
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

void Document::removeGraphTransform(int index)
{
    Q_ASSERT(index >= 0 && index < _graphTransformsModel.count());
    _graphTransformsModel.remove(index);
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

    QStringList newGraphTransforms = _graphTransforms;
    newGraphTransforms.move(from, to);

    _commandManager.execute(std::make_unique<ApplyTransformsCommand>(
        _graphModel.get(), _selectionManager.get(), this,
        _graphTransforms, newGraphTransforms));
}

QStringList Document::availableVisualisationChannelNames(int valueType) const
{
    return _graphModel != nullptr ? _graphModel->availableVisualisationChannelNames(
                                        static_cast<ValueType>(valueType)) : QStringList();
}

QString Document::visualisationDescription(const QString& attributeName, const QString& channelName) const
{
    return _graphModel != nullptr ? _graphModel->visualisationDescription(attributeName, channelName) : QString();
}

bool Document::hasVisualisationInfo() const
{
    return _graphModel != nullptr ? _graphModel->hasVisualisationInfo() : false;
}

QVariantMap Document::visualisationInfoAtIndex(int index) const
{
    QVariantMap map;

    map.insert("alertType", static_cast<int>(AlertType::None));
    map.insert("alertText", "");
    map.insert("minimumNumericValue", 0.0);
    map.insert("maximumNumericValue", 1.0);

    if(_graphModel == nullptr)
        return map;

    const auto& visualisationInfo = _graphModel->visualisationInfoAtIndex(index);

    map.insert("minimumNumericValue", visualisationInfo.min());
    map.insert("maximumNumericValue", visualisationInfo.max());

    auto alerts = visualisationInfo.alerts();

    if(alerts.empty())
        return map;

    std::sort(alerts.begin(), alerts.end(),
    [](auto& a, auto& b)
    {
        return a._type > b._type;
    });

    auto& alert = alerts.at(0);

    map.insert("alertType", static_cast<int>(alert._type));
    map.insert("alertText", alert._text);

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

void Document::removeVisualisation(int index)
{
    Q_ASSERT(index >= 0 && index < _visualisationsModel.count());
    _visualisationsModel.remove(index);
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

    QStringList newVisualisations = _visualisations;
    newVisualisations.move(from, to);

    _commandManager.execute(std::make_unique<ApplyVisualisationsCommand>(
        _graphModel.get(), this,
        _visualisations, newVisualisations));
}

void Document::update(QStringList newGraphTransforms, QStringList newVisualisations)
{
    if(_graphModel == nullptr)
        return;

    std::vector<std::unique_ptr<ICommand>> commands;

    auto uiGraphTransforms = graphTransformConfigurationsFromUI();

    if(!newGraphTransforms.empty())
    {
        for(const auto& newGraphTransform : newGraphTransforms)
        {
            if(!transformIsPinned(newGraphTransform))
            {
                // Insert before any existing pinned transforms
                int index = 0;
                while(index < uiGraphTransforms.size() && !transformIsPinned(uiGraphTransforms.at(index)))
                    index++;

                uiGraphTransforms.insert(index, newGraphTransform);
            }
            else
                uiGraphTransforms.append(newGraphTransform);
        }
    }

    if(transformsDiffer(_graphTransforms, uiGraphTransforms))
    {
        commands.emplace_back(std::make_unique<ApplyTransformsCommand>(
            _graphModel.get(), _selectionManager.get(), this,
            _graphTransforms, uiGraphTransforms));
    }
    else
        setTransforms(uiGraphTransforms);

    auto uiVisualisations = visualisationsFromUI();

    if(!newVisualisations.empty())
    {
        _graphModel->clearVisualisationInfos();
        uiVisualisations.append(newVisualisations);
    }

    if(visualisationsDiffer(_visualisations, uiVisualisations))
    {
        commands.emplace_back(std::make_unique<ApplyVisualisationsCommand>(
            _graphModel.get(), this,
            _visualisations, uiVisualisations));
    }
    else
        setVisualisations(uiVisualisations);

    if(commands.size() > 1)
    {
        _commandManager.execute({tr("Apply Transforms and Visualisations"),
                                 tr("Applying Transforms and Visualisations")},
                                std::move(commands));
    }
    else if(commands.size() == 1)
        _commandManager.execute(std::move(commands.front()));
}

QVariantMap Document::layoutSetting(const QString& name) const
{
    QVariantMap map;

    const auto* setting = _layoutThread->setting(name);
    if(setting != nullptr)
    {
        map.insert("name", setting->name());
        map.insert("displayName", setting->displayName());
        map.insert("value", setting->value());
        map.insert("minimumValue", setting->minimumValue());
        map.insert("maximumValue", setting->maximumValue());
    }

    return map;
}

void Document::setLayoutSettingValue(const QString& name, float value)
{
    _layoutThread->setSettingValue(name, value);
}

void Document::cancelCommand()
{
    _commandManager.cancel();
}

void Document::dumpGraph()
{
    _graphModel->graph().dumpToQDebug(2);
}
