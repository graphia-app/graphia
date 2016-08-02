#include "document.h"

#include "../application.h"

#include "shared/plugins/iplugin.h"

#include "../loading/parserthread.h"
#include "../graph/graphmodel.h"

#include "../layout/layout.h"
#include "../layout/forcedirectedlayout.h"
#include "../layout/collision.h"

#include "../commands/deleteselectednodescommand.h"
#include "../commands/applytransformationscommand.h"

#include "selectionmanager.h"
#include "graphquickitem.h"

REGISTER_QML_ENUM(LayoutPauseState);

Document::Document(QObject* parent) :
    QObject(parent)
{
}

bool Document::commandInProgress() const
{
    return !_loadComplete || _commandManager.busy();
}

bool Document::idle() const
{
    return !commandInProgress() && !_graphQuickItem->interacting();
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
    auto subPhase = _graphModel->graph().subPhase();

    if(!_loadComplete)
    {
        if(!phase.isEmpty())
            return QString(tr("Loading %1 (%2)").arg(_title).arg(phase));

        return QString(tr("Loading %1").arg(_title));
    }

    if(!phase.isEmpty())
    {
        if(!subPhase.isEmpty())
            return QString(tr("%1 (%2: %3)")).arg(_commandManager.commandVerb()).arg(phase).arg(subPhase);

        return QString(tr("%1 (%2)")).arg(_commandManager.commandVerb()).arg(phase);
    }

    return _commandManager.commandVerb();
}

void Document::updateLayoutState()
{
    if(idle() && !_userLayoutPaused)
        _layoutThread->resume();
    else
        _layoutThread->pauseAndWait();
}

LayoutPauseState::Enum Document::layoutPauseState()
{
    if(_layoutThread == nullptr)
        return LayoutPauseState::Enum::Paused;

    if(_userLayoutPaused)
        return LayoutPauseState::Enum::Paused;

    if(_layoutThread->finished())
        return LayoutPauseState::Enum::RunningFinished;

    return LayoutPauseState::Enum::Running;
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

bool Document::debugPauserEnabled() const
{
    return _graphModel != nullptr && _graphModel->graph().debugPauser.enabled();
}

bool Document::debugPaused() const
{
    return _graphModel != nullptr && _graphModel->graph().debugPauser.paused();
}

QString Document::debugResumeAction() const
{
    return _graphModel != nullptr ? _graphModel->graph().debugPauser.resumeAction() : tr("&Resume");
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

void Document::setTransforms(const std::vector<GraphTransformConfiguration>& transformations)
{
    _graphTransformConfigurations.setVector(transformsWithEmptyAppended(transformations));
    _previousGraphTransformConfigurations = transformations;
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

std::vector<GraphTransformConfiguration> Document::transformsWithEmptyAppended(
        const std::vector<GraphTransformConfiguration>& graphTransformConfigurations)
{
    bool allValid = std::all_of(graphTransformConfigurations.begin(), graphTransformConfigurations.end(),
    [](const GraphTransformConfiguration& graphTransformConfiguration)
    {
        return graphTransformConfiguration.valid();
    });

    std::vector<GraphTransformConfiguration> newGraphTransformConfigurations = graphTransformConfigurations;

    if(allValid)
        newGraphTransformConfigurations.emplace_back(this);

    return newGraphTransformConfigurations;
}

void Document::applyTransforms()
{
    if(_graphModel == nullptr)
        return;

    bool valid = std::all_of(_graphTransformConfigurations.vector().begin(),
                             _graphTransformConfigurations.vector().end(),
    [](auto& graphTransformConfiguration)
    {
        return graphTransformConfiguration.valid() ||
               graphTransformConfiguration.creationState() == GraphTransformCreationState::Enum::Uncreated;
    });

    if(!valid)
        return;

    auto transformCommand = std::make_shared<ApplyTransformationsCommand>(_graphModel.get(), _selectionManager.get(), this,
                                                                          _previousGraphTransformConfigurations,
                                                                          _graphTransformConfigurations.vector());

    _commandManager.execute(transformCommand);
}

void Document::onGraphTransformsConfigurationDataChanged(const QModelIndex& index,
                                                         const QModelIndex&,
                                                         const QVector<int>& roles)
{
    auto& graphTransformConfiguration = _graphTransformConfigurations.vector().at(index.row());
    auto& roleName = _graphTransformConfigurations.roleNames().value(roles.at(0));
    bool enabledChanging = (roleName == "transformEnabled");
    bool lockedChanging = (roleName == "locked");

    // Don't apply any changes when not enabled, as they will have no effect
    bool enabled = graphTransformConfiguration.enabled() || enabledChanging;

    if(graphTransformConfiguration.valid() && enabled && !lockedChanging)
        applyTransforms();
}

bool Document::openFile(const QUrl& fileUrl, const QString& fileType, const QString& pluginName)
{
    auto* plugin = _application->pluginForName(pluginName);
    if(plugin == nullptr)
        return false;

    setTitle(fileUrl.fileName());
    emit commandInProgressChanged();
    emit idleChanged();
    emit commandVerbChanged(); // Show Loading message

    _graphModel = std::make_shared<GraphModel>(fileUrl.fileName(), plugin);
    _selectionManager = std::make_shared<SelectionManager>(*_graphModel);

    _pluginInstance = plugin->createInstance();
    _pluginInstance->initialise(_graphModel.get(), _selectionManager.get());

    connect(&_graphModel->graph(), &Graph::phaseChanged, this, &Document::commandVerbChanged);

    connect(&_graphModel->graph().debugPauser, &DebugPauser::enabledChanged, this, &Document::debugPauserEnabledChanged);
    connect(&_graphModel->graph().debugPauser, &DebugPauser::pausedChanged, this, &Document::debugPausedChanged);
    connect(&_graphModel->graph().debugPauser, &DebugPauser::resumeActionChanged, this, &Document::debugResumeActionChanged);

    emit pluginInstanceChanged();

    auto parser = _pluginInstance->parserForUrlTypeName(fileType);
    if(parser == nullptr)
    {
        qDebug() << "Plugin does not provide parser";
        return false;
    }

    _graphFileParserThread = std::make_unique<ParserThread>(_graphModel->mutableGraph(), fileUrl, std::move(parser));
    connect(_graphFileParserThread.get(), &ParserThread::progress, this, &Document::onLoadProgress);
    connect(_graphFileParserThread.get(), &ParserThread::complete, this, &Document::onLoadComplete);
    _graphFileParserThread->start();

    return true;
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

    _layoutThread = std::make_unique<LayoutThread>(*_graphModel, std::make_unique<ForceDirectedLayoutFactory>(_graphModel));
    connect(_layoutThread.get(), &LayoutThread::pausedChanged, this, &Document::layoutPauseStateChanged);
    connect(_layoutThread.get(), &LayoutThread::settingChanged, this, &Document::updateLayoutState);
    _layoutThread->addAllComponents();
    _layoutSettings.setVectorPtr(&_layoutThread->settingsVector());

    _graphQuickItem->initialise(_graphModel, _commandManager, _selectionManager);

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

    connect(&_commandManager, &CommandManager::commandWillExecuteAsynchronously, _graphQuickItem, &GraphQuickItem::commandWillExecuteAsynchronously);
    connect(&_commandManager, &CommandManager::commandWillExecuteAsynchronously, this, &Document::commandInProgressChanged);

    connect(&_commandManager, &CommandManager::commandProgressChanged, this, &Document::commandProgressChanged);
    connect(&_commandManager, &CommandManager::commandVerbChanged, this, &Document::commandVerbChanged);

    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::canUndoChanged);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::nextUndoActionChanged);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::canRedoChanged);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::nextRedoActionChanged);
    connect(&_commandManager, &CommandManager::commandCompleted, [this](const Command*, const QString& pastParticiple)
    {
        setStatus(pastParticiple);
    });

    connect(&_commandManager, &CommandManager::commandCompleted, _graphQuickItem, &GraphQuickItem::commandCompleted);
    connect(&_commandManager, &CommandManager::commandCompleted, this, &Document::commandInProgressChanged);

    connect(_selectionManager.get(), &SelectionManager::selectionChanged, this, &Document::canDeleteChanged);

    connect(_layoutThread.get(), &LayoutThread::executed, _graphQuickItem, &GraphQuickItem::onLayoutChanged);

    connect(&_graphTransformConfigurations, &QmlContainerWrapperBase::dataChanged,
            this, &Document::onGraphTransformsConfigurationDataChanged);
    addGraphTransform();

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
        _commandManager.executeSynchronousOnce(
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

    if(_selectionManager)
    {
        _commandManager.executeSynchronousOnce(
            [this](Command&) { return _selectionManager->clearNodeSelection(); });
    }
}

void Document::invertSelection()
{
    if(!idle())
        return;

    if(_selectionManager)
    {
        _commandManager.executeOnce(tr("Invert Selection"), tr("Inverting Selection"),
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

void Document::toggleDebugPauser()
{
    _graphModel->graph().debugPauser.toggleEnabled();
}

void Document::debugResume()
{
    _graphModel->graph().debugPauser.resume();
}

QStringList Document::availableTransformNames() const
{
    return _graphModel != nullptr ? _graphModel->availableTransformNames() : QStringList();
}

QStringList Document::availableDataFields(const QString& transformName) const
{
    return _graphModel != nullptr ? _graphModel->availableDataFields(transformName) : QStringList();
}

const DataField& Document::dataFieldByName(const QString& dataFieldName) const
{
    static DataField nullDataField;
    return _graphModel != nullptr ? _graphModel->dataFieldByName(dataFieldName) : nullDataField;
}

DataFieldType Document::typeOfDataField(const QString& dataFieldName) const
{
    return _graphModel != nullptr ? _graphModel->typeOfDataField(dataFieldName) : DataFieldType::Unknown;
}

QStringList Document::avaliableConditionFnOps(const QString& dataFieldName) const
{
    return _graphModel != nullptr ? _graphModel->avaliableConditionFnOps(dataFieldName) : QStringList();
}

void Document::removeGraphTransform(int index)
{
    auto graphTransformConfigurations = _graphTransformConfigurations.vector();

    auto& graphTransformConfiguration = graphTransformConfigurations.at(index);
    bool needToRebuildTransforms = graphTransformConfiguration.valid() && graphTransformConfiguration.enabled();

    graphTransformConfigurations.erase(graphTransformConfigurations.begin() + index);

    _graphTransformConfigurations.setVector(transformsWithEmptyAppended(graphTransformConfigurations));

    if(needToRebuildTransforms)
        applyTransforms();
}

void Document::dumpGraph()
{
    _graphModel->graph().dumpToQDebug(2);
}
