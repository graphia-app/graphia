#ifndef BASEPARSER_H
#define BASEPARSER_H

#include "../loading/iparser.h"

#include <atomic>

class BaseParser : public IParser
{
private:
    std::atomic_bool _cancelAtomic;

public:
    void cancel() { _cancelAtomic = true; }

protected:
    bool cancelled() { return _cancelAtomic; }
};

#endif // BASEPARSER_H
