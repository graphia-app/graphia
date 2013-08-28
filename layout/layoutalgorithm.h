#ifndef LAYOUTALGORITHM_H
#define LAYOUTALGORITHM_H

#include "../graph/grapharray.h"

#include <QList>
#include <QMap>
#include <QVector3D>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QAtomicInt>

class LayoutAlgorithm : public QObject
{
    Q_OBJECT
private:
    QAtomicInt cancelAtomic;
    void setCancel(bool cancel)
    {
        int expectedValue = static_cast<int>(!cancel);
        int newValue = static_cast<int>(!!cancel);

        cancelAtomic.testAndSetRelaxed(expectedValue, newValue);
    }

    virtual void executeReal() = 0;

protected:
    const ReadOnlyGraph* _graph;
    NodeArray<QVector3D>* positions;
    int _iterations;

    bool shouldCancel()
    {
        return cancelAtomic.testAndSetRelaxed(1, 1);
    }

public:
    LayoutAlgorithm(const ReadOnlyGraph& graph, NodeArray<QVector3D>& positions, int defaultNumIterations = 1) :
        cancelAtomic(0),
        _graph(&graph),
        positions(&positions),
        _iterations(defaultNumIterations)
    {}

    const ReadOnlyGraph& graph() { return *_graph; }

    void execute()
    {
        setCancel(false);
        executeReal();
    }

    void cancel()
    {
        setCancel(true);
    }

    // Indicates that the algorithm is doing no useful work
    virtual bool shouldPause() { return false; }

    bool iterative() { return _iterations != 1; }

    const static int Unbounded = -1;
    int iterations() { return _iterations; }
    void setIterations(int _iterations) { this->_iterations = _iterations; }

signals:
    void progress(int percentage);
    void complete();
};

class LayoutThread : public QThread
{
    Q_OBJECT
private:
    QList<LayoutAlgorithm*> layoutAlgorithms;
    QMap<LayoutAlgorithm*, int> iterationsRemaining;
    QMutex mutex;
    bool _pause;
    bool _isPaused;
    bool _stop;
    QWaitCondition waitForPause;
    QWaitCondition waitForResume;

public:
    LayoutThread() :
        _pause(false), _isPaused(false), _stop(false)
    {}

    void add(LayoutAlgorithm* layoutAlgorithm)
    {
        QMutexLocker locker(&mutex);
        // Take ownership of the algorithm
        layoutAlgorithm->moveToThread(this);
        layoutAlgorithms.append(layoutAlgorithm);
        iterationsRemaining[layoutAlgorithm] = layoutAlgorithm->iterations();
    }

    void pause()
    {
        QMutexLocker locker(&mutex);
        _pause = true;

        for(LayoutAlgorithm* layoutAlgorithm : layoutAlgorithms)
            layoutAlgorithm->cancel();
    }

    void pauseAndWait()
    {
        QMutexLocker locker(&mutex);
        _pause = true;

        for(LayoutAlgorithm* layoutAlgorithm : layoutAlgorithms)
            layoutAlgorithm->cancel();

        waitForPause.wait(&mutex);
    }

    bool isPaused()
    {
        QMutexLocker locker(&mutex);
        return _isPaused;
    }

    void resume()
    {
        {
            QMutexLocker locker(&mutex);
            _pause = false;
            _isPaused = false;
        }

        waitForResume.wakeAll();
    }

    void stop()
    {
        {
            QMutexLocker locker(&mutex);
            _stop = true;
            _pause = false;

            for(LayoutAlgorithm* layoutAlgorithm : layoutAlgorithms)
                layoutAlgorithm->cancel();
        }

        waitForResume.wakeAll();
    }

private:
    bool workRemaining()
    {
        for(LayoutAlgorithm* layoutAlgorithm : layoutAlgorithms)
        {
            if(iterationsRemaining[layoutAlgorithm] != 0)
                return true;
        }

        return false;
    }

    bool allLayoutAlgorithmsShouldPause()
    {
        for(LayoutAlgorithm* layoutAlgorithm : layoutAlgorithms)
        {
            if(!layoutAlgorithm->shouldPause())
                return false;
        }

        return true;
    }

    void run() Q_DECL_OVERRIDE
    {
        while(workRemaining())
        {
            for(LayoutAlgorithm* layoutAlgorithm : layoutAlgorithms)
            {
                if(layoutAlgorithm->shouldPause())
                    continue;

                if(iterationsRemaining[layoutAlgorithm] != 0)
                    layoutAlgorithm->execute();

                if(iterationsRemaining[layoutAlgorithm] != LayoutAlgorithm::Unbounded)
                    iterationsRemaining[layoutAlgorithm]--;
            }

            {
                QMutexLocker locker(&mutex);

                if(_pause || allLayoutAlgorithmsShouldPause())
                {
                    _isPaused = true;
                    waitForPause.wakeAll();
                    waitForResume.wait(&mutex);
                }

                if(_stop)
                    break;
            }
        }

        for(LayoutAlgorithm* layoutAlgorithm : layoutAlgorithms)
            delete layoutAlgorithm;
    }
};

#endif // LAYOUTALGORITHM_H
