#include "commandmanager.h"

#include "shared/utils/utils.h"

#include <QDebug>

#include <thread>

CommandManager::CommandManager() :
    _lock(_mutex, std::defer_lock),
    _busy(false)
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

void CommandManager::undo()
{
    QMetaObject::invokeMethod(this, "undoReal");
}

void CommandManager::redo()
{
    QMetaObject::invokeMethod(this, "redoReal");
}

void CommandManager::executeReal(std::shared_ptr<Command> command, bool irreversible)
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

    command->setProgressFn([this](int progress) { _commandProgress = progress; emit commandProgressChanged(); });

    auto executeCommand = [this, command, irreversible]()
    {
        if(command->asynchronous())
            u::setCurrentThreadName(command->description());

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

        _busy = false;
        emit commandCompleted(command.get(), command->pastParticiple(), CommandAction::Execute);
    };

    _busy = true;
    emit busyChanged();
    if(_debug > 0)
        qDebug() << "Command started" << command->description();

    doCommand(command, command->verb(), executeCommand);
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

    auto undoCommand = [this, command]()
    {
        if(command->asynchronous())
            u::setCurrentThreadName("(u) " + command->description());

        command->undo();
        _lastExecutedIndex--;

        _busy = false;
        emit commandCompleted(command.get(), QString(), CommandAction::Undo);
    };

    _busy = true;
    emit busyChanged();

    doCommand(command, command->undoVerb(), undoCommand);
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

    auto redoCommand = [this, command]()
    {
        if(command->asynchronous())
            u::setCurrentThreadName("(r) " + command->description());

        command->execute();

        _busy = false;
        emit commandCompleted(command.get(), command->pastParticiple(), CommandAction::Redo);
    };

    _busy = true;
    emit busyChanged();

    doCommand(command, command->redoVerb(), redoCommand);
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
        return command->undoDescription();
    }

    return tr("Undo");
}

QString CommandManager::nextRedoAction() const
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);

    if(lock.owns_lock() && canRedoNoLocking())
    {
        auto& command = _stack.at(_lastExecutedIndex + 1);
        return command->redoDescription();
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

    _stack.clear();
    _lastExecutedIndex = -1;
}

bool CommandManager::canUndoNoLocking() const
{
    return _lastExecutedIndex >= 0;
}

bool CommandManager::canRedoNoLocking() const
{
    return _lastExecutedIndex < static_cast<int>(_stack.size()) - 1;
}

void CommandManager::onCommandCompleted(Command* command, const QString&, CommandAction)
{
    // If the command executed asynchronously, we need to join its thread
    if(_thread.joinable())
        _thread.join();

    Q_ASSERT(_lock.owns_lock());
    _lock.unlock();

    QString description;

    if(command != nullptr)
    {
        command->postExecute();
        description = command->description();
    }

    if(_debug > 0)
        qDebug() << "Command completed" << description;

    if(_deferredExecutor.hasTasks())
        _deferredExecutor.executeOne();
    else
        emit busyChanged();
}

void CommandManager::update()
{
    _deferredExecutor.execute();
}
