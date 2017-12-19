#ifndef IPARSER_H
#define IPARSER_H

#include "shared/utils/cancellable.h"
#include "progressfn.h"

class QUrl;
class IGraphModel;

class IParser : public Cancellable
{
public:
    ~IParser() override = default;

    virtual bool parse(const QUrl& url,
                       IGraphModel& graphModel,
                       const ProgressFn& progressFn) = 0;
};

#endif // IPARSER_H
