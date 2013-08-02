#ifndef LAYOUTALGORITHM_H
#define LAYOUTALGORITHM_H

#include "../graph/grapharray.h"

#include <QList>
#include <QVector3D>

class LayoutAlgorithm
{
protected:
    NodeArray<QVector3D>* positions;

public:
    LayoutAlgorithm(NodeArray<QVector3D>& positions) : positions(&positions) {}

    const Graph& graph() { return *positions->graph(); }
    virtual void execute() = 0;

    class ProgressListener
    {
    public:
        virtual void onProgress(int) const {}
        virtual void onCompletion() const {}
    };

protected:
    QList<const ProgressListener*> progressListeners;

public:
    void addProgressListener(const ProgressListener* progressListener)
    {
        progressListeners.append(progressListener);
    }

    void removeProgressListener(const ProgressListener* progressListener)
    {
        progressListeners.removeAll(progressListener);
    }

    void notifyListenersOfProgress(int percentage)
    {
        for(const ProgressListener* progressListener : progressListeners)
            progressListener->onProgress(percentage);
    }

    void notifyListenersOfCompletion()
    {
        for(const ProgressListener* progressListener : progressListeners)
            progressListener->onCompletion();
    }
};

#endif // LAYOUTALGORITHM_H
