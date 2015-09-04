#include "utils.h"

#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <memory>
#include <random>
#include <thread>

static std::random_device rd;
static std::mt19937 gen(rd());

float u::rand(float low, float high)
{
    std::uniform_real_distribution<> distribution(low, high);
    return distribution(gen);
}

int u::rand(int low, int high)
{
    std::uniform_int_distribution<> distribution(low, high);
    return distribution(gen);
}

QVector2D u::randQVector2D(float low, float high)
{
    return QVector2D(rand(low, high), rand(low, high));
}

QVector3D u::randQVector3D(float low, float high)
{
    return QVector3D(rand(low, high), rand(low, high), rand(low, high));
}

QColor u::randQColor()
{
    int r = rand(0, 255);
    int g = rand(0, 255);
    int b = rand(0, 255);

    return QColor(r, g, b);
}

float u::fast_rsqrt(float number)
{
    typedef union
    {
        float f;
        int i;
        unsigned int ui;
    } floatint_t;

    floatint_t t;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    t.f  = number;
    t.i  = 0x5f3759df -(t.i >> 1);               // what the fuck?
    y  = t.f;
    y  = y *(threehalfs -(x2 * y * y));   // 1st iteration
    //	y  = y *(threehalfs -(x2 * y * y));   // 2nd iteration, this can be removed

    return y;
}

QVector3D u::fastNormalize(const QVector3D& v)
{
    float rsqrtLen = fast_rsqrt(v.lengthSquared());
    return v * rsqrtLen;
}

int u::smallestPowerOf2GreaterThan(int x)
{
    if(x < 0)
        return 0;

    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

int u::currentThreadId()
{
    return static_cast<int>(std::hash<std::thread::id>()(std::this_thread::get_id()));
}

#if defined(__linux__)
#include <sys/prctl.h>
#include <unistd.h>
#include <sys/syscall.h>

void u::nameCurrentThread(const QString& name)
{
    if(syscall(SYS_gettid) != getpid()) // Avoid renaming main thread
        prctl(PR_SET_NAME, (char*)name.toUtf8().constData());
}

const QString u::currentThreadName()
{
    char threadName[16] = {0};
    prctl(PR_GET_NAME, (unsigned long)threadName);

    return threadName;
}

#elif _WIN32
#include <windows.h>
const DWORD MS_VC_EXCEPTION  =0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(char* threadName)
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = -1;
    info.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
}

void u::nameCurrentThread(const QString& name)
{
    SetThreadName(name.toLatin1().data());
}

const QString u::currentThreadName()
{
    // Windows doesn't really have a concept of named threads (see above), so use the ID instead
    return QString::number(Utils::currentThreadId());
}
#else
void u::nameCurrentThread(const QString&) {}
const QString u::currentThreadName()
{
    return QString::number(Utils::currentThreadId());
}
#endif

QQuaternion u::matrixToQuaternion(const QMatrix4x4& m)
{
    // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
    // article "Quaternion Calculus and Fast Animation".
    auto trace = m(0, 0) + m(1, 1) + m(2, 2);
    float w;
    float v[3];

    if(trace > 0.0f)
    {
        // |w| > 1/2, may as well choose w > 1/2
        auto root = std::sqrt(trace + 1.0f);  // 2w
        w = 0.5f * root;
        root = 0.5f/root;  // 1/(4w)
        v[0] = (m(1, 2) - m(2, 1)) * root;
        v[1] = (m(2, 0) - m(0, 2)) * root;
        v[2] = (m(0, 1) - m(1, 0)) * root;
    }
    else
    {
        // |w| <= 1/2
        int i = 0;

        if(m(1, 1) > m(0, 0))
          i = 1;

        if(m(2, 2) > m(i, i))
          i = 2;

        int next[3] = {1, 2, 0};
        int j = next[i];
        int k = next[j];

        auto root = std::sqrt(m(i, i) - m(j, j) - m(k, k) + 1.0f);
        v[i] = 0.5f * root;
        root = 0.5f / root;
        w = (m(j, k) - m(k, j)) * root;
        v[j] = (m(i, j) + m(j, i)) * root;
        v[k] = (m(i, k) + m(k, i)) * root;
    }

    return QQuaternion(w, v[0], v[1], v[2]);
}
