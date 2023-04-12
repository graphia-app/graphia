/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://www.hdfgroup.org/licenses.               *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "H5private.h"

H5_GCC_DIAG_OFF("larger-than=")
H5_CLANG_DIAG_OFF("overlength-strings")

const char H5build_settings[]=
    "       SUMMARY OF THE HDF5 CONFIGURATION\n"
    "       =================================\n"
    "\n"
    "General Information:\n"
    "-------------------\n"
    "                   HDF5 Version: 1.15.0\n"
    "                  Configured on: 2023-09-13\n"
    "                  Configured by: Visual Studio 16 2019\n"
    "                    Host system: Windows-10.0.19045\n"
    "              Uname information: Windows\n"
    "                       Byte sex: \n"
    "             Installation point: C:/Program Files/HDF_Group/HDF5/1.15.0\n"
    "\n"
    "Compiling Options:\n"
    "------------------\n"
    "                     Build Mode: $<CONFIG>\n"
    "              Debugging Symbols: OFF\n"
    "                        Asserts: OFF\n"
    "                      Profiling: OFF\n"
    "             Optimization Level: OFF\n"
    "\n"
    "Linking Options:\n"
    "----------------\n"
    "                      Libraries: \n"
    "  Statically Linked Executables: OFF\n"
    "                        LDFLAGS: /machine:x64\n"
    "                     H5_LDFLAGS: \n"
    "                     AM_LDFLAGS: \n"
    "                Extra libraries: shlwapi\n"
    "                       Archiver: C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.29.30133/bin/Hostx64/x64/lib.exe\n"
    "                       AR_FLAGS: \n"
    "                         Ranlib: :\n"
    "\n"
    "Languages:\n"
    "----------\n"
    "                              C: YES\n"
    "                     C Compiler: C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.29.30133/bin/Hostx64/x64/cl.exe 19.29.30151.0\n"
    "                       CPPFLAGS: \n"
    "                    H5_CPPFLAGS: \n"
    "                    AM_CPPFLAGS: \n"
    "                        C Flags:   /DWIN32 /D_WINDOWS -wd5105\n"
    "                     H5 C Flags: /W3;/wd4100;/wd4706;/wd4127\n"
    "                     AM C Flags: \n"
    "               Shared C Library: YES\n"
    "               Static C Library: YES\n"
    "\n"
    "\n"
    "                        Fortran: \n"
    "               Fortran Compiler:  \n"
    "                  Fortran Flags: \n"
    "               H5 Fortran Flags: \n"
    "               AM Fortran Flags: \n"
    "         Shared Fortran Library: YES\n"
    "         Static Fortran Library: YES\n"
    "               Module Directory: C:/Users/tim/sources/hdf5/build/mod\n"
    "\n"
    "                            C++: \n"
    "                   C++ Compiler:  \n"
    "                      C++ Flags: \n"
    "                   H5 C++ Flags: \n"
    "                   AM C++ Flags: \n"
    "             Shared C++ Library: YES\n"
    "             Static C++ Library: YES\n"
    "\n"
    "                           Java: \n"
    "                  Java Compiler:  \n"
    "\n"
    "\n"
    "Features:\n"
    "---------\n"
    "                     Parallel HDF5: OFF\n"
    "  Parallel Filtered Dataset Writes: \n"
    "                Large Parallel I/O: \n"
    "                High-level library: \n"
    "Dimension scales w/ new references: \n"
    "                  Build HDF5 Tests: \n"
    "                  Build HDF5 Tools: \n"
    "                   Build GIF Tools: \n"
    "                      Threadsafety: OFF\n"
    "               Default API mapping: v116\n"
    "    With deprecated public symbols: ON\n"
    "            I/O filters (external): \n"
    "                     Map (H5M) API: \n"
    "                        Direct VFD: \n"
    "                        Mirror VFD: \n"
    "                     Subfiling VFD: \n"
    "                (Read-Only) S3 VFD: \n"
    "              (Read-Only) HDFS VFD: \n"
    "    Packages w/ extra debug output: \n"
    "                       API tracing: OFF\n"
    "              Using memory checker: OFF\n"
    "            Function stack tracing: OFF\n"
    "                  Use file locking: best-effort\n"
    "         Strict file format checks: OFF\n"
    "      Optimization instrumentation: \n"
;

H5_GCC_DIAG_ON("larger-than=")
H5_CLANG_DIAG_OFF("overlength-strings")