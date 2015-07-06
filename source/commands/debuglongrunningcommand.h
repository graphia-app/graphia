#ifndef DEBUGLONGRUNNINGCOMMAND_H
#define DEBUGLONGRUNNINGCOMMAND_H

#include "command.h"

#include <condition_variable>
#include <mutex>

// This is a fake command that blocks until told to complete, for the purposes of
// debugging the state of things when a command is running
class DebugLongRunningCommand : public Command
{
private:
    static DebugLongRunningCommand* _activeLongRunningCommand;
    std::condition_variable _blocker;
    std::mutex _mutex;

public:
    DebugLongRunningCommand();

    bool execute();
    void undo();
    void cancel();

    static void stop();
    static bool running();
};

#endif // DEBUGLONGRUNNINGCOMMAND_H
