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
 * @file example_xlsxio_write.c
 * @brief XLSX I/O example illustrating how to write to an .xlsx file.
 * @author Brecht Sanders
 *
 * This example writes column headers and cells of different types to an .xslx file.
 */

#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "xlsxio_write.h"

const char* filename = "example.xlsx";

int main (int argc, char* argv[])
{
  xlsxiowriter handle;
  //open .xlsx file for writing (will overwrite if it already exists)
  if ((handle = xlsxiowrite_open(filename, "MySheet")) == NULL) {
    fprintf(stderr, "Error creating .xlsx file\n");
    return 1;
  }
  //set row height
  xlsxiowrite_set_row_height(handle, 1);
  //how many rows to buffer to detect column widths
  xlsxiowrite_set_detection_rows(handle, 10);
  //write column names
  xlsxiowrite_add_column(handle, "Col1", 0);
  xlsxiowrite_add_column(handle, "Col2", 21);
  xlsxiowrite_add_column(handle, "Col3", 0);
  xlsxiowrite_add_column(handle, "Col4", 2);
  xlsxiowrite_add_column(handle, "Col5", 0);
  xlsxiowrite_add_column(handle, "Col6", 0);
  xlsxiowrite_add_column(handle, "Col7", 0);
  xlsxiowrite_next_row(handle);
  //write data
  int i;
  for (i = 0; i < 1000; i++) {
    xlsxiowrite_add_cell_string(handle, "Test");
    xlsxiowrite_add_cell_string(handle, "A b  c   d    e     f\nnew line");
    xlsxiowrite_add_cell_string(handle, "&% <test> \"'");
    xlsxiowrite_add_cell_string(handle, NULL);
    xlsxiowrite_add_cell_int(handle, i);
    xlsxiowrite_add_cell_datetime(handle, time(NULL));
    xlsxiowrite_add_cell_float(handle, 3.1415926);
    xlsxiowrite_next_row(handle);
  }
  //close .xlsx file
  xlsxiowrite_close(handle);
  return 0;
}
