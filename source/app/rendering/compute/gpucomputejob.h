#ifndef GPUCOMPUTEJOB_H
#define GPUCOMPUTEJOB_H

#include "rendering/openglfunctions.h"

class GPUComputeJob : protected OpenGLFunctions
{
public:
    void initialise() { resolveOpenGLFunctions(); }

    ~GPUComputeJob() override = default;
    virtual void run() = 0;
};

#endif // GPUCOMPUTEJOB_H
