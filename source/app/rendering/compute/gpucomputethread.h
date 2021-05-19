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

#ifndef GPUCOMPUTE_H
#define GPUCOMPUTE_H

#include <thread>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <type_traits>

class GPUComputeJob;
class QOpenGLContext;
class QOffscreenSurface;
class QSurfaceFormat;

class GPUComputeThread
{
private:
    std::thread _thread;
    std::mutex _mutex;
    std::deque<std::unique_ptr<GPUComputeJob>> _jobs;
    std::condition_variable _jobsPending;
    std::condition_variable _jobCompleted;
    std::condition_variable _initialised;
    bool _shouldStop = false;

    QOpenGLContext* _mainGLContext = nullptr;
    std::unique_ptr<QOffscreenSurface> _surface;
    std::unique_ptr<QSurfaceFormat> _format;

public:
    GPUComputeThread();
    ~GPUComputeThread();
    void start();
    void stop();
    void destroySurface();
    void run();

    template<typename T> typename std::enable_if<std::is_base_of_v<GPUComputeJob, T>, void>::type
    enqueue(std::unique_ptr<T>& computeJob)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _jobs.push_back(std::move(computeJob));
        _jobsPending.notify_one();
    }

    void initialise();
    void clearJobs();
    void wait();
};

#endif // GPUCOMPUTE_H
