/*****************************************************************************
Copyright (C)  2016  Brecht Sanders  All Rights Reserved

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*****************************************************************************/

/**
 * @file example_xlsxio_write_getversion.c
 * @brief XLSX I/O example illustrating different methods of getting the version number.
 * @author Brecht Sanders
 *
 * This example gets the library version using the different methods available.
 * \sa     xlsxioread_get_version()
 * \sa     xlsxioread_get_version_string()
 * \sa     xlsxiowrite_get_version()
 * \sa     xlsxiowrite_get_version_string()
 * \sa     XLSXIO_VERSION_*
 * \sa     XLSXIO_VERSION_STRING
 */

#include <stdio.h>
#include "xlsxio_write.h"
#include "xlsxio_version.h"

int main (int argc, char* argv[])
{
  /* The following methods call the library to get information, this is the preferred method. */

  //get version string from library
  printf("Version: %s\n", xlsxiowrite_get_version_string());

  //get version numbers from library
  int major, minor, micro;
  xlsxiowrite_get_version(&major, &minor, &micro);
  printf("Version: %i.%i.%i\n", major, minor, micro);

  /* The following methods use header file xlsxio_version.h to get information, avoid when using shared libraries. */

  //get version string from header
  printf("Version: %s\n", XLSXIO_VERSION_STRING);

  //get version numbers from header
  printf("Version: %i.%i.%i\n", XLSXIO_VERSION_MAJOR, XLSXIO_VERSION_MINOR, XLSXIO_VERSION_MICRO);

  //get library name and version string from header
  printf("Name and version: %s\n", XLSXIOWRITE_FULLNAME);
  return 0;
}
