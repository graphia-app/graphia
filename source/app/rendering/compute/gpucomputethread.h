#ifndef GPUCOMPUTE_H
#define GPUCOMPUTE_H

#include "../openglfunctions.h"
#include "gpucomputejob.h"

#include <QObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOffscreenSurface>

#include <thread>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <type_traits>

class GPUComputeThread
{
private:
    std::thread _thread;
    std::mutex _mutex;
    std::deque<std::shared_ptr<GPUComputeJob>> _jobs;
    std::condition_variable _jobsPending;
    std::condition_variable _jobCompleted;
    std::condition_variable _initialised;
    bool _shouldStop = false;

    QOpenGLContext* _mainGLContext = nullptr;
    std::shared_ptr<QOpenGLContext> _computeGLContext;
    std::unique_ptr<QOffscreenSurface> _surface;
    std::unique_ptr<QSurfaceFormat> _format;

public:
    GPUComputeThread();
    ~GPUComputeThread();
    void start();
    void stop();
    void destroySurface();
    void run();

    template<typename T> typename std::enable_if<std::is_base_of<GPUComputeJob, T>::value, void>::type
    enqueue(std::shared_ptr<T> computeJob)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _jobs.push_back(computeJob);
        _jobsPending.notify_one();
    }

    void initialise();
    void clearJobs();
    void wait();
};

#endif // GPUCOMPUTE_H
