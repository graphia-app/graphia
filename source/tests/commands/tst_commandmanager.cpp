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

#include "tests/common/testenvironment.h"

#include "app/commands/commandmanager.h"
#include "app/preferences.h"

#include "shared/commands/command.h"

#include <QTest>
#include <QSignalSpy>
#include <QString>

#include <memory>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
// Sets a value, restoring the previous value when undone
class SetCommand : public ICommand
{
private:
    int* _value;
    int _newValue;
    int _oldValue = 0;
    bool _hasOldValue = false;
    bool _succeeds;

public:
    SetCommand(int& value, int newValue, bool succeeds = true) :
        _value(&value), _newValue(newValue), _succeeds(succeeds)
    {}

    QString description() const override { return u"Set %1"_s.arg(_newValue); }

    bool execute() override
    {
        if(!_succeeds)
            return false;

        if(!_hasOldValue)
        {
            _oldValue = *_value;
            _hasOldValue = true;
        }

        *_value = _newValue;
        return true;
    }

    void undo() override { *_value = _oldValue; }

    // Replacing a command takes on the value it replaced
    void replaces(const ICommand* replacee) override
    {
        const auto* setCommand = dynamic_cast<const SetCommand*>(replacee);
        _oldValue = setCommand->_oldValue;
        _hasOldValue = true;
    }
};

std::vector<QString> toVector(const std::vector<QString>& descriptions) { return descriptions; }
} // namespace

class TestCommandManager : public QObject
{
    Q_OBJECT

private:
    int _value = 0;
    std::unique_ptr<CommandManager> _commandManager;

    // Commands run on another thread, and are started from the event loop
    bool waitUntilIdle()
    {
        return QTest::qWaitFor([this] { return !_commandManager->busy(); }, 5000);
    }

    void set(int value, ExecutePolicy policy = ExecutePolicy::Add, bool succeeds = true)
    {
        _commandManager->execute(policy, std::make_unique<SetCommand>(_value, value, succeeds));
        QVERIFY(waitUntilIdle());
    }

    void undo()
    {
        _commandManager->undo();
        QVERIFY(waitUntilIdle());
    }

    void redo()
    {
        _commandManager->redo();
        QVERIFY(waitUntilIdle());
    }

    void doRollback()
    {
        _commandManager->rollback();
        QVERIFY(waitUntilIdle());
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void init()
    {
        _value = 0;
        _commandManager = std::make_unique<CommandManager>();
    }

    void cleanup()
    {
        _commandManager.reset();
        u::setPref(u"misc/maxUndoLevels"_s, 25);
    }

    void undoAndRedo()
    {
        QVERIFY(!_commandManager->canUndo());
        QVERIFY(!_commandManager->canRedo());

        set(1);
        set(2);
        QCOMPARE(_value, 2);
        QVERIFY(_commandManager->canUndo());
        QCOMPARE(_commandManager->nextUndoAction(), u"&Undo Set 2"_s);
        QCOMPARE(toVector(_commandManager->undoableCommandDescriptions()),
            (std::vector<QString>{u"Set 2"_s, u"Set 1"_s}));

        undo();
        QCOMPARE(_value, 1);
        QVERIFY(_commandManager->canRedo());
        QCOMPARE(_commandManager->nextRedoAction(), u"&Redo Set 2"_s);

        undo();
        QCOMPARE(_value, 0);
        QVERIFY(!_commandManager->canUndo());
        QCOMPARE(toVector(_commandManager->redoableCommandDescriptions()),
            (std::vector<QString>{u"Set 1"_s, u"Set 2"_s}));

        // Nothing to undo
        undo();
        QCOMPARE(_value, 0);

        redo();
        redo();
        QCOMPARE(_value, 2);
        QVERIFY(!_commandManager->canRedo());

        // Nothing to redo
        redo();
        QCOMPARE(_value, 2);
    }

    // Executing a command discards anything that could have been redone
    void executeAfterUndo()
    {
        set(1);
        set(2);
        undo();
        set(3);

        QCOMPARE(_value, 3);
        QVERIFY(!_commandManager->canRedo());
        QCOMPARE(toVector(_commandManager->undoableCommandDescriptions()),
            (std::vector<QString>{u"Set 3"_s, u"Set 1"_s}));

        undo();
        QCOMPARE(_value, 1);
    }

    void replace()
    {
        set(1);
        set(2);
        set(3, ExecutePolicy::Replace);

        QCOMPARE(_value, 3);
        QCOMPARE(toVector(_commandManager->undoableCommandDescriptions()),
            (std::vector<QString>{u"Set 3"_s, u"Set 1"_s}));

        undo();
        QCOMPARE(_value, 1);
    }

    void once()
    {
        set(1);
        set(2, ExecutePolicy::Once);

        QCOMPARE(_value, 2);
        QCOMPARE(toVector(_commandManager->undoableCommandDescriptions()),
            (std::vector<QString>{u"Set 1"_s}));
    }

    // A command that fails isn't added to the stack
    void failure()
    {
        set(1);

        const QSignalSpy spy(_commandManager.get(), &CommandManager::commandCompleted);
        set(2, ExecutePolicy::Add, false);

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toBool(), false);
        QCOMPARE(toVector(_commandManager->undoableCommandDescriptions()),
            (std::vector<QString>{u"Set 1"_s}));
    }

    void maxUndoLevels()
    {
        u::setPref(u"misc/maxUndoLevels"_s, 2);

        set(1);
        set(2);
        set(3);

        QCOMPARE(toVector(_commandManager->undoableCommandDescriptions()),
            (std::vector<QString>{u"Set 3"_s, u"Set 2"_s}));

        undo();
        undo();
        QCOMPARE(_value, 1);
        QVERIFY(!_commandManager->canUndo());
    }

    // A rollback is an undo that can't be redone
    void rollback()
    {
        set(1);
        set(2);
        doRollback();

        QCOMPARE(_value, 1);
        QVERIFY(!_commandManager->canRedo());
        QCOMPARE(toVector(_commandManager->undoableCommandDescriptions()),
            (std::vector<QString>{u"Set 1"_s}));
    }

    void clearCommandStack()
    {
        const QSignalSpy spy(_commandManager.get(), &CommandManager::commandStackCleared);

        set(1);
        set(2);
        undo();
        _commandManager->clearCommandStack();

        QCOMPARE(spy.count(), 1);
        QVERIFY(!_commandManager->canUndo());
        QVERIFY(!_commandManager->canRedo());
        QCOMPARE(_value, 1);
    }

    // Commands queued together run in order
    void queued()
    {
        for(int i = 1; i <= 5; i++)
            _commandManager->execute(ExecutePolicy::Add, std::make_unique<SetCommand>(_value, i));

        _commandManager->undo();
        _commandManager->undo();
        _commandManager->redo();

        QVERIFY(waitUntilIdle());
        QCOMPARE(_value, 4);
        QCOMPARE(toVector(_commandManager->redoableCommandDescriptions()), (std::vector<QString>{u"Set 5"_s}));
    }

    void commandFunctions()
    {
        QString log;

        _commandManager->execute(ExecutePolicy::Add, std::make_unique<Command>(
            Command::CommandDescription{u"Append"_s},
            [&log](Command&) { log += u"x"_s; },
            [&log](Command&) { log.chop(1); }));
        QVERIFY(waitUntilIdle());
        QCOMPARE(log, u"x"_s);

        undo();
        QCOMPARE(log, QString());

        _commandManager->executeOnce([&log](Command&) { log += u"y"_s; });
        QVERIFY(waitUntilIdle());
        QCOMPARE(log, u"y"_s);
    }

    void postCommandFunction()
    {
        int runs = 0;
        _commandManager->setPostCommandFunction([&runs] { runs++; });

        set(1);
        QCOMPARE(runs, 1);

        undo();
        QCOMPARE(runs, 2);

        _commandManager->schedulePostCommandFunction();
        QVERIFY(QTest::qWaitFor([&runs] { return runs == 3; }, 5000));

        _commandManager->clearPostCommandFunction();
    }
};

QTEST_GUILESS_MAIN(TestCommandManager)
#include "tst_commandmanager.moc"
