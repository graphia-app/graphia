#ifdef _WIN32
#include "H5pubconf_win.h"
#elif defined __APPLE__
#include "H5pubconf_macos.h"
#else
#include "H5pubconf_unix.h"
#endif

#undef ZLIB_DLL
