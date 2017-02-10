#include "commandmanager.h"

#include "shared/utils/utils.h"

#include <QDebug>

#include <thread>

CommandManager::CommandManager() :
    _lock(_mutex, std::defer_lock),
    _busy(false),
    _graphChanged(false)
{
    qRegisterMetaType<CommandAction>("CommandAction");
    connect(this, &CommandManager::commandQueued, this, &CommandManager::update);
    connect(this, &CommandManager::commandCompleted, this, &CommandManager::onCommandCompleted);

    _debug = qgetenv("COMMAND_DEBUG").toInt();
}

CommandManager::~CommandManager()
{
    if(_thread.joinable())
    {
        _currentCommand->cancel();
        _thread.join();
    }

    if(_lock.owns_lock())
        _lock.unlock();
}

void CommandManager::execute(const std::shared_ptr<ICommand>& command)
{
    _deferredExecutor.enqueue([this, command] { executeReal(command, false); });
    emit commandQueued();
}

void CommandManager::executeOnce(const std::shared_ptr<ICommand>& command)
{
    _deferredExecutor.enqueue([this, command] { executeReal(command, true); });
    emit commandQueued();
}

void CommandManager::undo()
{
    _deferredExecutor.enqueue([this] { undoReal(); });
    emit commandQueued();
}

void CommandManager::redo()
{
    _deferredExecutor.enqueue([this] { redoReal(); });
    emit commandQueued();
}

void CommandManager::executeReal(std::shared_ptr<ICommand> command, bool irreversible)
{
    if(_lock.owns_lock())
    {
        if(_debug > 0)
            qDebug() << "enqueuing command" << command->description();

        // Something is already executing, do the new command once that is finished
        _deferredExecutor.enqueue([this, command, irreversible] { executeReal(command, irreversible); });
        return;
    }

    _lock.lock();

    _busy = true;
    emit busyChanged();
    if(_debug > 0)
        qDebug() << "Command started" << command->description();

    doCommand(command, command->verb(), [this, command, irreversible]
    {
        u::setCurrentThreadName(command->description());

        _graphChanged = false;

        if(!command->execute())
        {
            _busy = false;
            emit commandCompleted(nullptr, QString(), CommandAction::None);
            return;
        }

        if(!irreversible)
        {
            // There are commands on the stack ahead of us; throw them away
            while(canRedoNoLocking())
                _stack.pop_back();

            _stack.push_back(command);
            _lastExecutedIndex = static_cast<int>(_stack.size()) - 1;
        }
        else if(_graphChanged)
        {
            // The graph changed during an irreversible command, so throw
            // away our redo history as it is likely no longer coherent with
            // the current state
            clearCommandStackNoLocking();
        }

        _busy = false;
        emit commandCompleted(command.get(), command->pastParticiple(), CommandAction::Execute);
    });
}

void CommandManager::undoReal()
{
    if(_lock.owns_lock())
    {
        _deferredExecutor.enqueue([this] { undoReal(); });
        return;
    }

    _lock.lock();

    if(!canUndoNoLocking())
        return;

    auto command = _stack.at(_lastExecutedIndex);

    _busy = true;
    emit busyChanged();

    QString undoVerb = !command->description().isEmpty() ?
                QObject::tr("Undoing ") + command->description() :
                QObject::tr("Undoing");

    doCommand(command, undoVerb, [this, command]
    {
        u::setCurrentThreadName("(u) " + command->description());

        command->undo();
        _lastExecutedIndex--;

        _busy = false;
        emit commandCompleted(command.get(), QString(), CommandAction::Undo);
    });
}

void CommandManager::redoReal()
{
    if(_lock.owns_lock())
    {
        _deferredExecutor.enqueue([this] { redoReal(); });
        return;
    }

    _lock.lock();

    if(!canRedoNoLocking())
        return;

    auto command = _stack.at(++_lastExecutedIndex);

    _busy = true;
    emit busyChanged();

    QString redoVerb = !command->description().isEmpty() ?
                QObject::tr("Redoing ") + command->description() :
                QObject::tr("Redoing");

    doCommand(command, redoVerb, [this, command]
    {
        u::setCurrentThreadName("(r) " + command->description());

        command->execute();

        _busy = false;
        emit commandCompleted(command.get(), command->pastParticiple(), CommandAction::Redo);
    });
}

bool CommandManager::canUndo() const
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);

    if(lock.owns_lock())
        return canUndoNoLocking();

    return false;
}

bool CommandManager::canRedo() const
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);

    if(lock.owns_lock())
        return canRedoNoLocking();

    return false;
}

const std::vector<QString> CommandManager::undoableCommandDescriptions() const
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);
    std::vector<QString> commandDescriptions;

    if(lock.owns_lock() && canUndoNoLocking())
    {
        for(int index = _lastExecutedIndex; index >= 0; index--)
            commandDescriptions.push_back(_stack.at(index)->description());
    }

    return commandDescriptions;
}

const std::vector<QString> CommandManager::redoableCommandDescriptions() const
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);
    std::vector<QString> commandDescriptions;

    if(lock.owns_lock() && canRedoNoLocking())
    {
        for(int index = _lastExecutedIndex + 1; index < static_cast<int>(_stack.size()); index++)
            commandDescriptions.push_back(_stack.at(index)->description());
    }

    return commandDescriptions;
}

QString CommandManager::nextUndoAction() const
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);

    if(lock.owns_lock() && canUndoNoLocking())
    {
        auto& command = _stack.at(_lastExecutedIndex);
        if(!command->description().isEmpty())
            return QObject::tr("Undo ") + command->description();
    }

    return tr("Undo");
}

QString CommandManager::nextRedoAction() const
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);

    if(lock.owns_lock() && canRedoNoLocking())
    {
        auto& command = _stack.at(_lastExecutedIndex + 1);
        if(!command->description().isEmpty())
            return QObject::tr("Redo ") + command->description();
    }

    return tr("Redo");
}

bool CommandManager::busy() const
{
    return _busy;
}

void CommandManager::clearCommandStack()
{
    std::unique_lock<std::mutex> lock(_mutex);

    // If a command is still in progress, wait until it's finished
    if(_thread.joinable())
        _thread.join();

    clearCommandStackNoLocking();
}

void CommandManager::clearCommandStackNoLocking()
{
    _stack.clear();
    _lastExecutedIndex = -1;

    // Force a UI update
    emit commandStackCleared();
}

void CommandManager::timerEvent(QTimerEvent*)
{
    int newCommandProgress = _currentCommand->progress();

    if(newCommandProgress != _commandProgress)
    {
        _commandProgress = newCommandProgress;
        emit commandProgressChanged();
    }
}

bool CommandManager::canUndoNoLocking() const
{
    return _lastExecutedIndex >= 0;
}

bool CommandManager::canRedoNoLocking() const
{
    return _lastExecutedIndex < static_cast<int>(_stack.size()) - 1;
}

void CommandManager::onCommandCompleted(ICommand* command, const QString&, CommandAction)
{
    killTimer(_commandProgressTimerId);
    _commandProgressTimerId = -1;

    if(_thread.joinable())
        _thread.join();

    Q_ASSERT(_lock.owns_lock());
    _lock.unlock();

    QString description = command != nullptr ? command->description() : "";

    if(_debug > 0)
        qDebug() << "Command completed" << description;

    if(_deferredExecutor.hasTasks())
        _deferredExecutor.executeOne();
    else
        emit busyChanged();
}

void CommandManager::update()
{
    _deferredExecutor.executeOne();
}
