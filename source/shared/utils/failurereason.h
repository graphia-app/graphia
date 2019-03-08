#ifndef FAILUREREASON_H
#define FAILUREREASON_H

#include <QString>

class FailureReason
{
private:
    QString _failureReason;

public:
    virtual ~FailureReason() = default;

    void setFailureReason(const QString& failureReason) { _failureReason = failureReason; }
    const QString& failureReason() const { return _failureReason; }
};

#endif // FAILUREREASON_H
