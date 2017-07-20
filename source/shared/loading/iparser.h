#ifndef IPARSER_H
#define IPARSER_H

#include "shared/utils/cancellable.h"
#include "progressfn.h"

class QUrl;
class IMutableGraph;

class IParser : public Cancellable
{
public:
    virtual ~IParser() = default;

    virtual bool parse(const QUrl& url,
                       IMutableGraph& mutableGraph,
                       const ProgressFn& progressFn) = 0;
};

#endif // IPARSER_H
