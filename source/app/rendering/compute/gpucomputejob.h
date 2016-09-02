#ifndef GPUCOMPUTEJOB_H
#define GPUCOMPUTEJOB_H

#include "../openglfunctions.h"

class GPUComputeJob : protected OpenGLFunctions
{
public:
    virtual ~GPUComputeJob() {}
    virtual void run() = 0;
};

#endif // GPUCOMPUTEJOB_H
