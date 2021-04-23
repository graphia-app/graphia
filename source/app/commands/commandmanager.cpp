/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

#include "commandmanager.h"

#include "shared/utils/thread.h"
#include "shared/utils/preferences.h"

#include <QDebug>

#include <thread>

CommandManager::CommandManager() :
    _graphChanged(false)
{
    connect(this, &CommandManager::commandQueued, this, &CommandManager::update);
    connect(this, &CommandManager::commandCompleted, this, &CommandManager::onCommandCompleted);

    _debug = qEnvironmentVariableIntValue("COMMAND_DEBUG");
}

CommandManager::~CommandManager()
{
    if(_thread.joinable())
    {
        if(_currentCommand != nullptr)
            _currentCommand->cancel();

        joinThread();
    }
}

void CommandManager::execute(ExecutePolicy policy, ICommandPtr command)
{
    CommandAction action;

    switch(policy)
    {
    default:
    case ExecutePolicy::Add:        action = CommandAction::Execute; break;
    case ExecutePolicy::Replace:    action = CommandAction::ExecuteReplace; break;

    case ExecutePolicy::OnceMutate: command->_notAllowedToChangeGraph = false;
        [[fallthrough]];
    case ExecutePolicy::Once:       action = CommandAction::ExecuteOnce; break;
    }

    std::unique_lock<std::recursive_mutex> lock(_queueMutex);
    _pendingCommands.emplace_back(action, std::move(command));
    emit commandQueued();
}

void CommandManager::undo()
{
    std::unique_lock<std::recursive_mutex> lock(_queueMutex);
    _pendingCommands.emplace_back(CommandAction::Undo);
    emit commandQueued();
}

void CommandManager::redo()
{
    std::unique_lock<std::recursive_mutex> lock(_queueMutex);
    _pendingCommands.emplace_back(CommandAction::Redo);
    emit commandQueued();
}

void CommandManager::rollback()
{
    std::unique_lock<std::recursive_mutex> lock(_queueMutex);
    _pendingCommands.emplace_back(CommandAction::Rollback);
    emit commandQueued();
}

static void commandStartDebug(int debug, bool busy, const QString& verb)
{
    if(debug > 0 && !busy)
        qDebug() << "CommandManager started";

    if(debug > 1)
    {
        if(verb.isEmpty())
            qDebug() << "Command started" << verb;
        else
            qDebug() << "Command started";
    }
}

void CommandManager::executeReal(ICommandPtr command, CommandAction action)
{
    Q_ASSERT(action == CommandAction::Execute ||
        action == CommandAction::ExecuteReplace ||
        action == CommandAction::ExecuteOnce);

    commandStartDebug(_debug, _busy, command->description());

    auto* commandPtr = command.get();
    auto verb = command->verb();
    doCommand(commandPtr, verb, [this, command = std::move(command), action]() mutable
    {
        std::unique_lock<std::recursive_mutex> lock(_mutex);

        QString commandName = command->description().length() > 0 ?
            command->description() : QStringLiteral("Anon Command");
        u::setCurrentThreadName(commandName);

        _graphChanged = false;

        QString description;
        QString pastParticiple;
        bool success = false;

        if(action == CommandAction::ExecuteReplace && lastExecutedCommand() != nullptr)
            command->replaces(lastExecutedCommand());

        if(command->execute() && !command->cancelled())
        {
            description = command->description();
            pastParticiple = command->pastParticiple();
            success = true;

            if(action != CommandAction::ExecuteOnce)
            {
                // There are commands on the stack ahead of us; throw them away
                while(canRedoNoLocking())
                    _stack.pop_back();

                if(!_stack.empty() && action == CommandAction::ExecuteReplace)
                    _stack.pop_back();

                _stack.push_back(std::move(command));

                auto maxUndoLevels = u::pref("misc/maxUndoLevels").toInt();
                if(maxUndoLevels > 0)
                {
                    // Lose commands at the bottom of the stack until
                    // we reach the maximum stack size
                    while(static_cast<int>(_stack.size()) > maxUndoLevels)
                        _stack.pop_front();
                }

                _lastExecutedIndex = static_cast<int>(_stack.size()) - 1;
            }
            else if(command->_notAllowedToChangeGraph && _graphChanged)
            {
                // The graph changed during an irreversible command, so throw
                // away our redo history as it is likely no longer coherent with
                // the current state
                clearCommandStackNoLocking();
                qDebug() << "WARNING: ExecuteOnce command" << commandName <<
                    "changed state, so the command stack was cleared";
            }
        }

        clearCurrentCommand();

        emit commandCompleted(success, description, pastParticiple);
    });
}

void CommandManager::undoReal(bool rollback)
{
    if(!canUndoNoLocking())
        return;

    auto* command = _stack.at(_lastExecutedIndex).get();

    QString undoVerb = !command->description().isEmpty() ?
                QObject::tr("Undoing ") + command->description() :
                QObject::tr("Undoing");

    if(rollback)
    {
        // Rollbacks are silent to the user
        undoVerb.clear();
    }

    commandStartDebug(_debug, _busy, undoVerb);

    doCommand(command, undoVerb, [this, command, rollback]
    {
        std::unique_lock<std::recursive_mutex> lock(_mutex);

        u::setCurrentThreadName("(u) " + command->description());
        auto description = QObject::tr("Undo %1").arg(command->description());

        command->undo();
        _lastExecutedIndex--;

        clearCurrentCommand();

        if(rollback)
        {
            // If rolling back, prevent redo
            _stack.pop_back();
        }

        emit commandCompleted(true, description, {});
    });
}

void CommandManager::redoReal()
{
    if(!canRedoNoLocking())
        return;

    auto* command = _stack.at(++_lastExecutedIndex).get();

    QString redoVerb = !command->description().isEmpty() ?
                QObject::tr("Redoing ") + command->description() :
                QObject::tr("Redoing");

    commandStartDebug(_debug, _busy, redoVerb);

    doCommand(command, redoVerb, [this, command]
    {
        std::unique_lock<std::recursive_mutex> lock(_mutex);

        u::setCurrentThreadName("(r) " + command->description());
        auto description = QObject::tr("Redo %1").arg(command->description());

        command->execute();

        clearCurrentCommand();

        emit commandCompleted(true, description, command->pastParticiple());
    });
}

bool CommandManager::canUndo() const
{
    std::unique_lock<std::recursive_mutex> lock(_mutex, std::try_to_lock);

    if(lock.owns_lock())
        return canUndoNoLocking();

    return false;
}

bool CommandManager::canRedo() const
{
    std::unique_lock<std::recursive_mutex> lock(_mutex, std::try_to_lock);

    if(lock.owns_lock())
        return canRedoNoLocking();

    return false;
}

bool CommandManager::commandIsCancellable() const
{
    std::unique_lock<std::mutex> lock(_currentCommandMutex);

    if(_currentCommand != nullptr)
        return _currentCommand->cancellable();

    return false;
}

std::vector<QString> CommandManager::undoableCommandDescriptions() const
{
    std::unique_lock<std::recursive_mutex> lock(_mutex, std::try_to_lock);
    std::vector<QString> commandDescriptions;
    commandDescriptions.reserve(_lastExecutedIndex);

    if(lock.owns_lock() && canUndoNoLocking())
    {
        for(int index = _lastExecutedIndex; index >= 0; index--)
            commandDescriptions.push_back(_stack.at(index)->description());
    }

    return commandDescriptions;
}

std::vector<QString> CommandManager::redoableCommandDescriptions() const
{
    std::unique_lock<std::recursive_mutex> lock(_mutex, std::try_to_lock);
    std::vector<QString> commandDescriptions;
    commandDescriptions.reserve(_stack.size());

    if(lock.owns_lock() && canRedoNoLocking())
    {
        for(int index = _lastExecutedIndex + 1; index < static_cast<int>(_stack.size()); index++)
            commandDescriptions.push_back(_stack.at(index)->description());
    }

    return commandDescriptions;
}

QString CommandManager::nextUndoAction() const
{
    std::unique_lock<std::recursive_mutex> lock(_mutex, std::try_to_lock);

    if(lock.owns_lock() && canUndoNoLocking())
    {
        const auto& command = _stack.at(_lastExecutedIndex);
        if(!command->description().isEmpty())
            return QObject::tr("Undo ") + command->description();
    }

    return tr("Undo");
}

QString CommandManager::nextRedoAction() const
{
    std::unique_lock<std::recursive_mutex> lock(_mutex, std::try_to_lock);

    if(lock.owns_lock() && canRedoNoLocking())
    {
        const auto& command = _stack.at(static_cast<size_t>(_lastExecutedIndex) + 1);
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
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    // If a command is still in progress, wait until it's finished
    joinThread();

    clearCommandStackNoLocking();
}

void CommandManager::clearCommandStackNoLocking()
{
    _stack.clear();
    _lastExecutedIndex = -1;

    // Force a UI update
    emit commandStackCleared();
}

void CommandManager::clearCurrentCommand()
{
    // _currentCommand is a raw pointer, so we must ensure it is reset to null
    // when the underlying unique_ptr is destroyed
    std::unique_lock<std::mutex> lock(_currentCommandMutex);
    _currentCommand = nullptr;

    bool wasCancelling = _cancelling;
    _cancelling = false;

    if(wasCancelling)
        emit commandIsCancellingChanged();
}


void CommandManager::cancel()
{
    std::unique_lock<std::mutex> lock(_currentCommandMutex);

    if(_currentCommand != nullptr)
    {
        _cancelling = true;
        emit commandIsCancellingChanged();
        _currentCommand->cancel();

        if(_debug > 0)
            qDebug() << "Command cancel request" << _currentCommand->description();
    }
}

void CommandManager::wait()
{
    do
    {
        joinThread();
    }
    while(commandsArePending());
}

QString CommandManager::commandStackSummary() const
{
    // Strictly speaking this should be locked, but since
    // it is only ever called in a setting where we're
    // crashed, that could cause problems

    QString text;
    int index = 0;

    for(const auto& command : _stack)
    {
        if(!text.isEmpty())
            text.append("\n");

        if(_lastExecutedIndex == index++)
            text.append("*");

        text.append(command->debugDescription());
    }

    if(_currentCommand != nullptr)
    {
        text.append(QStringLiteral("\n#%1").arg(
            _currentCommand->debugDescription()));
    }

    return text;
}

void CommandManager::joinThread()
{
    if(_thread.joinable())
    {
        _thread.join();
        _threadActive = false;
    }
}

void CommandManager::timerEvent(QTimerEvent*)
{
    std::unique_lock<std::mutex> lock(_currentCommandMutex);

    if(_currentCommand == nullptr)
        return;

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

const ICommand* CommandManager::lastExecutedCommand() const
{
    if(_stack.empty() || _lastExecutedIndex < 0)
        return nullptr;

    return _stack.at(_lastExecutedIndex).get();
}

bool CommandManager::commandsArePending() const
{
    std::unique_lock<std::recursive_mutex> lock(_queueMutex);

    return !_pendingCommands.empty();
}

CommandManager::PendingCommand CommandManager::nextPendingCommand()
{
    std::unique_lock<std::recursive_mutex> lock(_queueMutex);

    if(_pendingCommands.empty())
        return {};

    auto pendingCommand = std::move(_pendingCommands.front());
    _pendingCommands.pop_front();

    return pendingCommand;
}

void CommandManager::onCommandCompleted(bool success, const QString& description, const QString&)
{
    killTimer(_commandProgressTimerId);
    _commandProgressTimerId = -1;

    joinThread();

    if(_debug > 1)
    {
        if(success)
        {
            if(!description.isEmpty())
                qDebug() << "Command completed" << description;
            else
                qDebug() << "Command completed";
        }
        else
            qDebug() << "Command failed/cancelled";
    }

    if(commandsArePending())
        update();
    else
    {
        _busy = false;
        emit finished();

        if(_debug > 0)
            qDebug() << "CommandManager finished";

        emit commandIsCancellableChanged();
    }
}

void CommandManager::update()
{
    if(_threadActive || !commandsArePending())
        return;

    auto pendingCommand = nextPendingCommand();
    switch(pendingCommand._action)
    {
    case CommandAction::Execute:
    case CommandAction::ExecuteReplace:
    case CommandAction::ExecuteOnce:
        executeReal(std::move(pendingCommand._command), pendingCommand._action);
        break;

    case CommandAction::Undo:       undoReal(); break;
    case CommandAction::Redo:       redoReal(); break;
    case CommandAction::Rollback:   undoReal(true); break;
    default: break;
    }
}

void CommandManager::onGraphChanged(const Graph*, bool changeOccurred)
{
    if(changeOccurred)
        _graphChanged = true;
}
