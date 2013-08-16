#ifndef LAYOUTALGORITHM_H
#define LAYOUTALGORITHM_H

#include "../graph/grapharray.h"

#include <QList>
#include <QVector3D>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

class LayoutAlgorithm : public QObject
{
    Q_OBJECT
protected:
    NodeArray<QVector3D>* positions;
    int _iterations;

public:
    LayoutAlgorithm(NodeArray<QVector3D>& positions, int defaultNumIterations) :
        positions(&positions),
        _iterations(defaultNumIterations)
    {}

    Graph& graph() { return *positions->graph(); }
    virtual void execute() = 0;

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
    LayoutAlgorithm* layoutAlgorithm;
    QMutex mutex;
    bool _pause;
    bool _isPaused;
    bool _stop;
    QWaitCondition waitForPause;
    QWaitCondition waitForResume;

public:
    LayoutThread(LayoutAlgorithm* layoutAlgorithm) :
        layoutAlgorithm(layoutAlgorithm),
        _pause(false), _isPaused(false), _stop(false)
    {
        // Take ownership of the algorithm
        layoutAlgorithm->moveToThread(this);
    }

    void pause()
    {
        QMutexLocker locker(&mutex);
        _pause = true;
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
        QMutexLocker locker(&mutex);
        _stop = true;
    }

private:
    void run() Q_DECL_OVERRIDE
    {
        for(int i = 0; i < layoutAlgorithm->iterations() ||
            layoutAlgorithm->iterations() == LayoutAlgorithm::Unbounded; i++)
        {
            layoutAlgorithm->execute();

            {
                QMutexLocker locker(&mutex);
                if(_stop)
                    break;

                if(_pause)
                {
                    _isPaused = true;
                    waitForPause.wakeAll();
                    waitForResume.wait(&mutex);
                }
            }
        }

        delete layoutAlgorithm;
    }
};

#endif // LAYOUTALGORITHM_H
