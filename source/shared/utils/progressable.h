#ifndef PROGRESSABLE_H
#define PROGRESSABLE_H

#include <functional>

using ProgressFn = std::function<void(int)>;

class Progressable
{
private:
    ProgressFn _progressFn = [](int){};

public:
    virtual ~Progressable() = default;

    virtual void setProgress(int percent) { _progressFn(percent); }
    void setProgressFn(const ProgressFn& f) { _progressFn = f; }
};

#endif // PROGRESSABLE_H
