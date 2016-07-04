#ifndef BASEPARSER_H
#define BASEPARSER_H

#include "../loading/iparser.h"

#include <atomic>

class BaseParser : public IParser
{
private:
    std::atomic_bool _cancelAtomic{false};

public:
    void cancel() { _cancelAtomic = true; }
    bool cancelled() const { return _cancelAtomic; }
};

#endif // BASEPARSER_H
