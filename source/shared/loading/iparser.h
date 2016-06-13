#ifndef IPARSER_H
#define IPARSER_H

#include <functional>

class QUrl;
class IMutableGraph;

class IParser
{
public:
    virtual ~IParser() = default;

    using ProgressFn = std::function<void(int)>;

    virtual bool parse(const QUrl& url,
                       IMutableGraph& mutableGraph,
                       const ProgressFn& progressReportFn) = 0;
    virtual void cancel() = 0;
};

#endif // IPARSER_H
