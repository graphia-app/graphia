/* Copyright © 2013-2021 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gpucomputethread.h"
#include "gpucomputejob.h"

#include "shared/utils/thread.h"

#include <QOffscreenSurface>
#include <QSurfaceFormat>
#include <QOpenGLContext>

GPUComputeThread::GPUComputeThread() : // NOLINT modernize-use-equals-default
    _surface(std::make_unique<QOffscreenSurface>()),
    _format(std::make_unique<QSurfaceFormat>(QSurfaceFormat::defaultFormat()))
{
    _surface->setFormat(*_format);

    // GUI main
    _surface->create();
}

GPUComputeThread::~GPUComputeThread()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _shouldStop = true;
    lock.unlock();
    _jobsPending.notify_one();

    if(_thread.joinable())
        _thread.join();
}

void GPUComputeThread::start()
{
    _shouldStop = false;
    _thread = std::thread(&GPUComputeThread::run, this);
}

void GPUComputeThread::stop()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _shouldStop = true;
}

void GPUComputeThread::destroySurface()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _shouldStop = true;
    _jobs.clear();
    _surface.reset();
    _format.reset();
}

void GPUComputeThread::clearJobs()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _jobs.clear();
}

void GPUComputeThread::wait()
{
    std::unique_lock<std::mutex> lock(_mutex);

    while(!_jobs.empty())
        _jobCompleted.wait(lock);
}

void GPUComputeThread::run()
{
    std::unique_lock<std::mutex> lock(_mutex);

    u::setCurrentThreadName(QStringLiteral("ComputeThread"));

    QOpenGLContext computeGLContext;
    computeGLContext.setShareContext(_mainGLContext);

    computeGLContext.setFormat(*_format);
    computeGLContext.create();

    computeGLContext.makeCurrent(_surface.get());

    _initialised.notify_one();

    while(!_shouldStop)
    {
        if(_jobs.empty())
            _jobsPending.wait(lock);

        if(_shouldStop)
            break;

        while(!_jobs.empty())
        {
            auto& job = _jobs.front();
            job->initialise();
            job->run();
            _jobs.pop_front();
            _jobCompleted.notify_one();
        }
    }
}

void GPUComputeThread::initialise()
{
    // Prevent multiple initialisation
    Q_ASSERT(_mainGLContext == nullptr);

    _mainGLContext = QOpenGLContext::currentContext();
    auto* surface = _mainGLContext->surface();

    // Release context from current thread
    _mainGLContext->doneCurrent();

    // Start the thread and wait until it's initialised
    std::unique_lock<std::mutex> lock(_mutex);
    start();
    _initialised.wait(lock);

    // Restore the context we saved earlier
    _mainGLContext->makeCurrent(surface);
}
