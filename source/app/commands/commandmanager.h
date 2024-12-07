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

#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include "shared/commands/icommandmanager.h"
#include "shared/utils/fatalerror.h"
#include "shared/utils/namedbool.h"

#include <QtGlobal>
#include <QObject>
#include <QString>

#include <functional>
#include <deque>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>

class Graph;

class CommandManager : public QObject, public ICommandManager
{
    Q_OBJECT

    Q_PROPERTY(int commandProgress MEMBER _commandProgress NOTIFY commandProgressChanged)
    Q_PROPERTY(QString commandPhase MEMBER _commandPhase NOTIFY commandPhaseChanged)
    Q_PROPERTY(QString commandVerb MEMBER _commandVerb NOTIFY commandVerbChanged)
    Q_PROPERTY(bool commandIsCancellable READ commandIsCancellable NOTIFY commandIsCancellableChanged)
    Q_PROPERTY(bool commandIsCancelling MEMBER _cancelling NOTIFY commandIsCancellingChanged)

public:
    CommandManager();
    ~CommandManager() override;

    void clear();

    void execute(ExecutePolicy policy, ICommandPtr command) override;
    using ICommandManager::execute;

    void setPhase(const QString& phase) override;

    void undo();
    void redo();

    // A rollback is just an undo that can't be redone
    void rollback();

    bool canUndo() const;
    bool canRedo() const;

    int commandProgress() const { return _commandProgress; }
    QString commandPhase() const { return _commandPhase; }
    QString commandVerb() const { return _commandVerb; }
    bool commandIsCancellable() const;
    bool commandIsCancelling() const { return _cancelling; }

    std::vector<QString> undoableCommandDescriptions() const;
    std::vector<QString> redoableCommandDescriptions() const;

    QString nextUndoAction() const;
    QString nextRedoAction() const;

    bool busy() const;

    void clearCommandStack();

    void cancel();

    void wait();

    // This is intended for debugging only
    QString commandStackSummary() const;

private:
    template<typename Fn> void doCommand(ICommand* command,
                                         const QString& verb, Fn&& fn)
    {
        command->initialise();

        _currentCommand = command;
        _commandProgress = -1;
        _commandVerb = verb;
        _commandPhase.clear();
        _threadActive = true;
        emit commandProgressChanged();
        emit commandPhaseChanged();
        emit commandVerbChanged();
        emit commandIsCancellableChanged();

        _commandProgressTimerId = startTimer(200);

        // If the command thread is still joinable, we shouldn't be here
        Q_ASSERT(!_thread.joinable());

        if(!_busy)
        {
            _busy = true;
            emit started();

            // The thread REALLY shouldn't be joinable yet
            Q_ASSERT(!_thread.joinable());
        }

        _thread = std::thread(std::forward<Fn>(fn));
    }

    void joinThread();

    void clearCurrentCommand();

    void timerEvent(QTimerEvent *event) override;

    bool canUndoNoLocking() const;
    bool canRedoNoLocking() const;
    const ICommand* lastExecutedCommand() const;

    enum class CommandAction
    {
        Execute,
        ExecuteReplace,
        ExecuteOnce,
        Undo,
        Redo,
        Rollback
    };

    void executeReal(ICommandPtr command, CommandAction action);
    void undoReal(NamedBool<"rollback"> rollback = "rollback"_no);
    void redoReal();

    void clearCommandStackNoLocking();

    struct PendingCommand
    {
        PendingCommand(CommandAction action, ICommandPtr command) :
            _action(action), _command(std::move(command))
        {
            if((action == CommandAction::Execute ||
                action == CommandAction::ExecuteReplace ||
                action == CommandAction::ExecuteOnce) &&
                _command == nullptr)
            {
                FATAL_ERROR(NullPendingCommand);
            }
        }

        explicit PendingCommand(CommandAction action) :
            _action(action), _command(nullptr) {}

        PendingCommand() = default;

        CommandAction _action = CommandAction::Execute;
        ICommandPtr _command = nullptr;
    };

    bool commandsArePending() const;
    PendingCommand nextPendingCommand();

    std::deque<PendingCommand> _pendingCommands;
    std::deque<ICommandPtr> _stack;
    int _lastExecutedIndex = -1;

    std::thread _thread;
    bool _threadActive = false;
    mutable std::recursive_mutex _mutex;
    mutable std::recursive_mutex _queueMutex;

    bool _busy = false;
    std::atomic_bool _graphChanged;

    mutable std::recursive_mutex _currentCommandMutex;
    ICommand* _currentCommand = nullptr;
    int _commandProgressTimerId = -1;
    int _commandProgress = 0;
    QString _commandVerb;
    QString _commandPhase;
    bool _cancelling = false;

    int _debug = 0;

private slots:
    void onCommandCompleted(bool success, const QString& description, const QString& pastParticiple);
    void update();

public slots:
    void onGraphChanged(const Graph*, bool changeOccurred);

signals:
    void started();

    void commandProgressChanged();
    void commandVerbChanged();
    void commandPhaseChanged();
    void commandIsCancellableChanged();
    void commandIsCancellingChanged();
    void commandQueued();
    void commandCompleted(bool success, const QString& description, const QString& pastParticiple);
    void commandStackCleared();

    void finished();
};

#endif // COMMANDMANAGER_H
