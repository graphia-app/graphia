#ifndef IPARSER_H
#define IPARSER_H

#include "shared/utils/progressable.h"
#include "shared/utils/cancellable.h"

class QUrl;
class IGraphModel;

class IParser : public Progressable, public Cancellable
{
public:
    ~IParser() override = default;

    virtual bool parse(const QUrl& url, IGraphModel* graphModel = nullptr) = 0;
};

#endif // IPARSER_H
