#ifndef DEBUGLONGRUNNINGCOMMAND_H
#define DEBUGLONGRUNNINGCOMMAND_H

#include "command.h"

#include <condition_variable>
#include <mutex>

class Graph;

// This is a fake command that blocks until told to complete, for the purposes of
// debugging the state of things when a command is running
class DebugLongRunningCommand : public Command
{
private:
    static DebugLongRunningCommand* _activeLongRunningCommand;
    std::condition_variable _blocker;
    std::mutex _mutex;

    Graph* _graph;
    bool _doGraphChange;

public:
    DebugLongRunningCommand(Graph* graph, bool doGraphChange = false);

    bool execute();
    void undo();
    void cancel();

    static void stop();
    static bool running();
};

#endif // DEBUGLONGRUNNINGCOMMAND_H
