#include "utils.h"

#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <memory>
#include <random>
#include <thread>
#include <array>

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

#include <QFile>

void u::setCurrentThreadName(const QString& name)
{
    if(syscall(SYS_gettid) != getpid()) // Avoid renaming main thread
        prctl(PR_SET_NAME, static_cast<const char*>(name.toUtf8().constData()));
}

const QString u::currentThreadName()
{
    char threadName[16] = {0};
    prctl(PR_GET_NAME, reinterpret_cast<uint64_t>(threadName));

    return QString(threadName);
}

QString u::parentProcessName()
{
    auto ppid = getppid();

    QFile procFile(QString("/proc/%1/cmdline").arg(ppid));
    if(procFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream ts(&procFile);
        QString commandLine = ts.readLine();
        auto tokens = commandLine.split(QRegExp(QString(QChar(0))));

        if(!tokens.empty())
            return tokens.at(0);
    }

    return {};
}

#elif defined(_WIN32) && !defined(__MINGW32__)
#include <windows.h>
#include <tlhelp32.h>
#include <Psapi.h>

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

static void SetThreadName(char* threadName)
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

void u::setCurrentThreadName(const QString& name)
{
    SetThreadName(name.toLatin1().data());
}

const QString u::currentThreadName()
{
    // Windows doesn't really have a concept of named threads (see above), so use the ID instead
    return QString::number(u::currentThreadId());
}

QString u::parentProcessName()
{
    PROCESSENTRY32 pe = {0};
    DWORD ppid = 0;
    pe.dwSize = sizeof(PROCESSENTRY32);
    HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(Process32First(handle, &pe))
    {
        do
        {
            if(pe.th32ProcessID == GetCurrentProcessId())
            {
                ppid = pe.th32ParentProcessID;
                break;
            }
        } while(Process32Next(handle, &pe));
    }
    CloseHandle(handle);

    HANDLE processHandle = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE,
        ppid);

    if(processHandle)
    {
        TCHAR charBuffer[MAX_PATH];
        if(GetModuleFileNameEx(processHandle, 0, charBuffer, MAX_PATH))
            return QString::fromWCharArray(charBuffer);

        CloseHandle(processHandle);
    }

    return {};
}
#else
void u::setCurrentThreadName(const QString&) {}
const QString u::currentThreadName()
{
    return QString::number(u::currentThreadId());
}
QString u::parentProcessName() { return {}; }
#endif

QQuaternion u::matrixToQuaternion(const QMatrix4x4& m)
{
    // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
    // article "Quaternion Calculus and Fast Animation".
    auto trace = m(0, 0) + m(1, 1) + m(2, 2);
    float w;
    std::array<float, 3> v;

    if(trace > 0.0f)
    {
        // |w| > 1/2, may as well choose w > 1/2
        auto root = std::sqrt(trace + 1.0f);  // 2w
        w = 0.5f * root;
        root = 0.5f/root;  // 1/(4w)
        v.at(0) = (m(1, 2) - m(2, 1)) * root;
        v.at(1) = (m(2, 0) - m(0, 2)) * root;
        v.at(2) = (m(0, 1) - m(1, 0)) * root;
    }
    else
    {
        // |w| <= 1/2
        int i = 0;

        if(m(1, 1) > m(0, 0))
          i = 1;

        if(m(2, 2) > m(i, i))
          i = 2;

        std::array<int, 3> next{{1, 2, 0}};
        int j = next.at(i);
        int k = next.at(j);

        auto root = std::sqrt(m(i, i) - m(j, j) - m(k, k) + 1.0f);
        v.at(i) = 0.5f * root;
        root = 0.5f / root;
        w = (m(j, k) - m(k, j)) * root;
        v.at(j) = (m(i, j) + m(j, i)) * root;
        v.at(k) = (m(i, k) + m(k, i)) * root;
    }

    return QQuaternion(w, v.at(0), v.at(1), v.at(2));
}

bool u::isNumeric(const std::string& string)
{
    std::size_t pos;
    long double value = 0.0;

    try
    {
        value = std::stold(string, &pos);
    }
    catch(std::invalid_argument&)
    {
        return false;
    }
    catch(std::out_of_range&)
    {
        return false;
    }

    return pos == string.size() && !std::isnan(value);
}

QColor u::contrastingColor(const QColor& color)
{

    float brightness = 0.299f * color.redF() +
                       0.587f * color.greenF() +
                       0.114f * color.blueF();
    float blackDiff = std::abs(brightness - 0.0f);
    float whiteDiff = std::abs(brightness - 1.0f);

    return (blackDiff > whiteDiff) ? Qt::black : Qt::white;
}
