#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "xlsxio_write.h"
#include "xlsxio_read.h"

const char* filename = "example.xlsx";

struct row_data {
  char* col1;
  char* col2;
  int64_t col3;
  double col4;
  time_t col5;
};

void set_row_data (struct row_data* data, char* col1, char* col2, long long col3, double col4, time_t col5)
{
  data->col1 = col1;
  data->col2 = col2;
  data->col3 = col3;
  data->col4 = col4;
  data->col5 = col5;
}

void clean_up_row_data (struct row_data* data)
{
  free(data->col1);
  free(data->col2);
}

void printf_row_data (struct row_data* data)
{
  printf("%s|%s|%" PRIi64 "|%.32G|%lu\n", (data->col1 ? data->col1 : ""), (data->col2 ? data->col2 : ""), data->col3, data->col4, (unsigned long)data->col5);
}

void write_row_data (xlsxiowriter handle, struct row_data* data)
{
  xlsxiowrite_add_cell_string(handle, data->col1);
  xlsxiowrite_add_cell_string(handle, data->col2);
  xlsxiowrite_add_cell_int(handle, data->col3);
  xlsxiowrite_add_cell_float(handle, data->col4);
  xlsxiowrite_add_cell_datetime(handle, data->col5);
  xlsxiowrite_next_row(handle);
}

int read_row_data (xlsxioreadersheet handle, struct row_data* data)
{
  int result;
  if ((result = xlsxioread_sheet_next_row(handle)) != 0) {
    xlsxioread_sheet_next_cell_string(handle, &data->col1);
    xlsxioread_sheet_next_cell_string(handle, &data->col2);
    xlsxioread_sheet_next_cell_int(handle, &data->col3);
    xlsxioread_sheet_next_cell_float(handle, &data->col4);
    xlsxioread_sheet_next_cell_datetime(handle, &data->col5);
  }
  return result;
}

int main (int argc, char* argv[])
{
  xlsxiowriter dst;
  xlsxioreader src;
  xlsxioreadersheet srcsheet;

  //remove the destination file first if it exists
  unlink(filename);
  //open .xlsx file for writing
  if ((dst = xlsxiowrite_open(filename, "MySheet")) == NULL) {
    fprintf(stderr, "Error creating .xlsx file\n");
    return 1;
  }
  //write data
  struct row_data data;
  xlsxiowrite_add_cell_string(dst, "Col1");
  xlsxiowrite_add_cell_string(dst, "Col2");
  xlsxiowrite_add_cell_string(dst, "Col3");
  xlsxiowrite_add_cell_string(dst, "Col4");
  xlsxiowrite_add_cell_string(dst, "Col5");
  xlsxiowrite_next_row(dst);

  //first row
  set_row_data(&data, NULL, strdup("Test"), 123, 3.1415926, time(NULL));
  printf_row_data(&data);
  write_row_data(dst, &data);
  clean_up_row_data(&data);
  //second row
  set_row_data(&data, strdup("Test"), strdup("123"), 456, 6.666666666666, time(NULL));
  printf_row_data(&data);
  write_row_data(dst, &data);
  clean_up_row_data(&data);
  //close .xlsx file
  xlsxiowrite_close(dst);

  printf("\n");

  //open .xlsx file for reading
  if ((src = xlsxioread_open (filename)) == NULL) {
    fprintf(stderr, "Error opening .xlsx file\n");
    return 1;
  }
  //open sheet
  if ((srcsheet = xlsxioread_sheet_open(src, NULL, XLSXIOREAD_SKIP_EMPTY_ROWS)) != NULL) {
    //read data
    while (read_row_data(srcsheet, &data)) {
      printf_row_data(&data);
      clean_up_row_data(&data);
    }
    //close sheet
    xlsxioread_sheet_close(srcsheet);
  }
  //close .xlsx file
  xlsxioread_close(src);
  return 0;
}
