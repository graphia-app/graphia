#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <QObject>
#include <QStack>
#include <QString>
#include <QList>

class Command : public QObject
{
    friend class CommandManager;

    Q_OBJECT
public:
    Command(const QString& description);

    const QString& description();

private:
    virtual void execute() = 0;
    virtual void undo() = 0;

    QString _description;
};

class CommandManager : public QObject
{
    Q_OBJECT
public:
    CommandManager();
    ~CommandManager();

    void clear();
    void execute(Command* command);

    void undo();
    void redo();

    bool canUndo() const;
    bool canRedo() const;

    const QList<const Command*> undoableCommands() const;
    const QList<const Command*> redoableCommands() const;

private:
    QStack<Command*> _stack;
    int _lastExecutedIndex;
};

#endif // COMMANDMANAGER_H
