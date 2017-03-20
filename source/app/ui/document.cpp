#include "document.h"

#include "application.h"

#include "shared/plugins/iplugin.h"
#include "shared/utils/preferences.h"
#include "shared/utils/flags.h"

#include "graph/graphmodel.h"
#include "loading/parserthread.h"

#include "layout/forcedirectedlayout.h"
#include "layout/layout.h"
#include "layout/collision.h"

#include "commands/deleteselectednodescommand.h"
#include "commands/applytransformationscommand.h"
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
    float brightness = 0.299f * backColor.redF()
                       + 0.587f * backColor.greenF()
                       + 0.114f * backColor.blueF();
    float blackDiff = std::abs(brightness - 0.0f);
    float whiteDiff = std::abs(brightness - 1.0f);

    return (blackDiff > whiteDiff) ? Qt::black : Qt::white;
}

Document::Document(QObject* parent) :
    QObject(parent),
    _graphChanging(false),
    _graphTransformsModel(this)
{}

Document::~Document()
{
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

void Document::updateLayoutState()
{
    if(idle() && !_userLayoutPaused)
        _layoutThread->resume();
    else
        _layoutThread->pauseAndWait();
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
        appendGraphTransform(transform);
}

void Document::setVisualisations(const QStringList& visualisations)
{
    _visualisations = visualisations;

    _visualisationsModel.clear();
    for(const auto& visualisation : visualisations)
        appendVisualisation(visualisation);
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

QStringList Document::graphTransformConfigurationsFromUI() const
{
    QStringList transforms;

    for(const auto& variant : _graphTransformsModel.list())
        transforms.append(variant.toString());

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

QStringList Document::visualisationsFromUI() const
{
    QStringList visualisations;

    for(const auto& variant : _visualisationsModel.list())
        visualisations.append(variant.toString());

    return visualisations;
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

    _graphModel = std::make_shared<GraphModel>(fileUrl.fileName(), plugin);

    _gpuComputeThread = std::make_shared<GPUComputeThread>();
    _graphFileParserThread = std::make_unique<ParserThread>(_graphModel->mutableGraph(), fileUrl);

    _selectionManager = std::make_shared<SelectionManager>(*_graphModel);
    _searchManager = std::make_shared<SearchManager>(*_graphModel);

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
        _graphModel->buildTransforms(_pluginInstance->defaultTransforms());
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

    setTransforms(_pluginInstance->defaultTransforms());
    setVisualisations(_pluginInstance->defaultVisualisations());

    _layoutThread = std::make_unique<LayoutThread>(*_graphModel, std::make_unique<ForceDirectedLayoutFactory>(_graphModel));
    connect(_layoutThread.get(), &LayoutThread::pausedChanged, this, &Document::layoutPauseStateChanged);
    connect(_layoutThread.get(), &LayoutThread::settingChanged, this, &Document::updateLayoutState);
    _layoutThread->addAllComponents();
    _layoutSettings.setVectorPtr(&_layoutThread->settingsVector());

    _graphQuickItem->initialise(_graphModel, _commandManager, _selectionManager, _gpuComputeThread);

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

    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::canUndoChanged);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::nextUndoActionChanged);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::canRedoChanged);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::nextRedoActionChanged);
    connect(&_commandManager, &CommandManager::commandCompleted, [this](const ICommand*, const QString& pastParticiple)
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
            _graphModel.get(), &GraphModel::onSelectionChanged, Qt::DirectConnection);

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
    {
        _graphChanging = false;
        maybeEmitIdleChanged();

        // If the graph has changed outside of a Command, then our new state is
        // inconsistent wrt the CommandManager, so throw away our undo history
        if(!commandInProgress())
            _commandManager.clearCommandStack();

        // If the graph changes then so do our visualisations
        _graphModel->buildVisualisations(_visualisations);
    });

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
        _commandManager.executeOnce(
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
        _commandManager.executeOnce(
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

    _commandManager.execute(std::make_shared<DeleteSelectedNodesCommand>(_graphModel.get(), _selectionManager.get()));
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
    int previousNumNodesFound = numNodesFound();

    _searchManager->findNodes(regex);

    if(previousNumNodesFound != numNodesFound())
        emit numNodesFoundChanged();
}

static bool shouldMoveFindFocus(bool inOverviewMode)
{
    return u::pref("misc/focusFoundNodes").toBool() &&
        ((inOverviewMode && u::pref("misc/focusFoundComponents").toBool()) || !inOverviewMode);
}

void Document::selectFirstFound()
{
    setFoundIt(_foundNodeIds.begin());
    _commandManager.executeOnce(makeSelectNodeCommand(_selectionManager.get(), *_foundIt));

    if(shouldMoveFindFocus(_graphQuickItem->inOverviewMode()))
        _graphQuickItem->moveFocusToNode(*_foundIt);
}

void Document::selectNextFound()
{
    incrementFoundIt();
    _commandManager.executeOnce(makeSelectNodeCommand(_selectionManager.get(), *_foundIt));

    if(shouldMoveFindFocus(_graphQuickItem->inOverviewMode()))
        _graphQuickItem->moveFocusToNode(*_foundIt);
}

void Document::selectPrevFound()
{
    decrementFoundIt();
    _commandManager.executeOnce(makeSelectNodeCommand(_selectionManager.get(), *_foundIt));

    if(shouldMoveFindFocus(_graphQuickItem->inOverviewMode()))
        _graphQuickItem->moveFocusToNode(*_foundIt);
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

void Document::onMutableGraphChanged()
{
    // This is only called in order to force the UI to refresh the transform
    // controls, in case the attribute ranges have changed
    setTransforms(_graphTransforms);
}

void Document::executeDeferred()
{
    _deferredExecutor.execute();
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

void Document::incrementFoundIt()
{
    if(_foundItValid && std::next(_foundIt) != _foundNodeIds.end())
        ++_foundIt;
    else
        _foundIt = _foundNodeIds.begin();

    emit foundIndexChanged();
}

void Document::decrementFoundIt()
{
    if(_foundItValid && _foundIt != _foundNodeIds.begin())
        --_foundIt;
    else
        _foundIt = std::prev(_foundNodeIds.end());

    emit foundIndexChanged();
}

void Document::executeOnMainThread(DeferredExecutor::TaskFn task, const QString& description)
{
    _deferredExecutor.enqueue(task, description);
    emit taskAddedToExecutor();
}

QStringList Document::availableTransformNames() const
{
    return _graphModel != nullptr ? _graphModel->availableTransformNames() : QStringList();
}

QStringList Document::availableAttributes(int elementTypes, int valueTypes) const
{
    return _graphModel != nullptr ? _graphModel->availableAttributes(
                                        static_cast<ElementType>(elementTypes),
                                        static_cast<ValueType>(valueTypes)) : QStringList();
}

QStringList Document::availableAttributesFor(const QString& transformName) const
{
    return _graphModel != nullptr ? _graphModel->availableAttributesFor(transformName) : QStringList();
}

QStringList Document::availableAttributesSimilarTo(const QString& attributeName) const
{
    if(_graphModel == nullptr)
        return {};

    const auto& attribute = _graphModel->attributeByName(attributeName);
    auto valueType = Flags<ValueType>(attribute.valueType());

    // For similarity purposes, treat Int and Float as the same
    if(valueType.anyOf(ValueType::Int, ValueType::Float))
        valueType.set(ValueType::Int, ValueType::Float);

    return _graphModel->availableAttributes(attribute.elementType(), *valueType);
}

QStringList Document::avaliableConditionFnOps(const QString& attributeName) const
{
    return _graphModel != nullptr ? _graphModel->avaliableConditionFnOps(attributeName) : QStringList();
}

QVariantMap Document::attribute(const QString& attributeName) const
{
    QVariantMap map;

    if(_graphModel == nullptr)
        return map;

    if(u::contains(_graphModel->availableAttributes(), attributeName))
    {
        const auto& attribute = _graphModel->attributeByName(attributeName);
        map.insert("valueType", static_cast<int>(attribute.valueType()));
        map.insert("elementType", static_cast<int>(attribute.elementType()));

        map.insert("hasRange", attribute.hasFloatRange() || attribute.hasIntRange());
        map.insert("hasMinimumValue", attribute.hasFloatMin() || attribute.hasIntMin());
        map.insert("hasMaximumValue", attribute.hasFloatMax() || attribute.hasIntMax());

        if(attribute.hasFloatMin()) map.insert("minimumValue", attribute.floatMin());
        if(attribute.hasFloatMax()) map.insert("maximumValue", attribute.floatMax());
        if(attribute.hasIntMin()) map.insert("minimumValue", attribute.intMin());
        if(attribute.hasIntMax()) map.insert("maximumValue", attribute.intMax());

        map.insert("description", attribute.description());
    }

    return map;
}

QVariantMap Document::findTransformParameter(const QString& transformName, const QString& parameterName) const
{
    if(_graphModel == nullptr)
        return {};

    if(u::contains(_graphModel->availableAttributesFor(transformName), parameterName))
    {
        // It's an Attribute
        return attribute(parameterName);
    }
    /*else
    {
        //FIXME it's a with ... parameter
    }*/

    return {};
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

void Document::appendGraphTransform(const QString& transform)
{
    if(!graphTransformIsValid(transform))
    {
        qDebug() << QString("Failed to parse transform '%1'").arg(transform);
        return;
    }

    if(!transformIsPinned(transform))
    {
        // Insert before any existing pinned transforms
        int index = 0;
        while(index < _graphTransformsModel.count() && !transformIsPinned(_graphTransformsModel.get(index).toString()))
            index++;

        _graphTransformsModel.insert(index, transform);
    }
    else
        _graphTransformsModel.append(transform);
}

void Document::removeGraphTransform(int index)
{
    Q_ASSERT(index >= 0 && index < _graphTransformsModel.count());
    _graphTransformsModel.remove(index);
}

template<typename Config>
static bool FlagsDiffer(const Config& a,
                                 const Config& b,
                                 const char* Flag)
{
    bool aResult = a.isFlagSet(Flag);
    bool bResult = b.isFlagSet(Flag);

    return aResult != bResult;
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

        if(FlagsDiffer(ai, bi, "disabled"))
            return true;

        if(FlagsDiffer(ai, bi, "repeating"))
            return true;

        if(ai != bi)
            return true;
    }

    return false;
}

void Document::updateGraphTransforms()
{
    if(_graphModel == nullptr)
        return;

    auto newGraphTransforms = graphTransformConfigurationsFromUI();

    if(transformsDiffer(_graphTransforms, newGraphTransforms))
    {
        _commandManager.execute(std::make_shared<ApplyTransformationsCommand>(
            _graphModel.get(), _selectionManager.get(), this,
            _graphTransforms, newGraphTransforms));
    }
    else
        setTransforms(newGraphTransforms);
}

QStringList Document::availableVisualisationChannelNames(const QString& attributeName) const
{
    return _graphModel != nullptr ? _graphModel->availableVisualisationChannelNames(attributeName) : QStringList();
}

QString Document::visualisationDescription(const QString& attributeName, const QString& channelName) const
{
    return _graphModel != nullptr ? _graphModel->visualisationDescription(attributeName, channelName) : QString();
}

QVariantMap Document::visualisationAlertAtIndex(int index) const
{
    QVariantMap map;

    map.insert("type", static_cast<int>(VisualisationAlertType::None));
    map.insert("text", "");

    if(_graphModel == nullptr)
        return map;

    auto visualisationAlerts = _graphModel->visualisationAlertsAtIndex(index);

    if(visualisationAlerts.empty())
        return map;

    std::sort(visualisationAlerts.begin(), visualisationAlerts.end(),
    [](auto& a, auto& b)
    {
        return a._type > b._type;
    });

    auto& visualisationAlert = visualisationAlerts.at(0);

    map.insert("type", static_cast<int>(visualisationAlert._type));
    map.insert("text", visualisationAlert._text);

    return map;
}

QVariantMap Document::parseVisualisation(const QString& visualisation) const
{
    VisualisationConfigParser p;
    if(p.parse(visualisation))
        return p.result().asVariantMap();

    return {};
}

bool Document::visualisationIsValid(const QString& visualisation) const
{
    return _graphModel != nullptr ? _graphModel->visualisationIsValid(visualisation) : false;
}

void Document::appendVisualisation(const QString& visualisation)
{
    _visualisationsModel.append(visualisation);
}

void Document::removeVisualisation(int index)
{
    Q_ASSERT(index >= 0 && index < _visualisationsModel.count());
    _visualisationsModel.remove(index);
}

// This tests two transform lists to determine if replacing one with the
// other would actually result in a different transformation
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

        if(FlagsDiffer(ai, bi, "disabled"))
            return true;

        if(FlagsDiffer(ai, bi, "invert"))
            return true;

        if(ai != bi)
            return true;
    }

    return false;
}

void Document::updateVisualisations()
{
    if(_graphModel == nullptr)
        return;

    auto newVisualisations = visualisationsFromUI();

    if(visualisationsDiffer(_visualisations, newVisualisations))
    {
        _commandManager.execute(std::make_shared<ApplyVisualisationsCommand>(
            _graphModel.get(), this,
            _visualisations, newVisualisations));
    }
    else
        setVisualisations(newVisualisations);
}

void Document::dumpGraph()
{
    _graphModel->graph().dumpToQDebug(2);
}
