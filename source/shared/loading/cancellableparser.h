#ifndef CANCELLABLEPARSER_H
#define CANCELLABLEPARSER_H

#include "../loading/iparser.h"

#include <QDebug>

#include <atomic>

class CancellableParser : public virtual ICancellableParser
{
private:
    std::atomic_bool _cancelAtomic;

public:
    void reset() { _cancelAtomic = false; }
    void cancel() { _cancelAtomic = true; }

protected:
    bool cancelled() { return _cancelAtomic; }
};

#endif // CANCELLABLEPARSER_H
