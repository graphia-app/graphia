#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "xlsxio_write.h"

/*! \class XLSXIOWriter
 *  \brief class for writing data to an .xlsx file
 *\details C++ wrapper for xlsxiowrite_ functions.
 */
class XLSXIOWriter
{
 private:
  xlsxiowriter handle;
 public:

  /*! \brief XLSXIOWriter constructor, creates and opens .xlsx file
   * \param  filename      path of .xlsx file to open
   * \param  sheetname     name of worksheet
   * \param  detectionrows number of rows to buffer in memory, zero for none, defaults to 5
   * \sa     xlsxiowrite_open()
   */
  XLSXIOWriter (const char* filename, const char* sheetname = NULL, size_t detectionrows = 5);

  /*! \brief XLSXIOWriter destructor, closes .xlsx file
   * \sa     xlsxiowrite_close()
   */
  ~XLSXIOWriter ();

  /*! \brief specify the row height to use for the current and next rows
   * \param  height        row height (in text lines), zero for unspecified
   * Must be called before the first call to any Add method of the current row
   * \sa     xlsxiowrite_set_row_height()
   */
  void SetRowHeight (size_t height = 0);

  /*! \brief add a column cell
   * \param  name          column name
   * \param  width         column width (in characters)
   * Only one row of column names is supported or none.
   * Call for each column, and finish column row by calling NextRow().
   * Must be called before any NextRow() or the AddCell methods.
   * \sa     NextRow()
   */
  void AddColumn (const char* name, int width = 0);

  /*! \brief add a cell with string data
   * \param  value         string value
   * \sa     NextRow()
   */
  void AddCellString (const char* value);

  /*! \brief add a cell with integer data
   * \param  value         integer value
   * \sa     NextRow()
   */
  void AddCellInt (long long value);

  /*! \brief add a cell with floating point data
   * \param  value         floating point value
   * \sa     NextRow()
   */
  void AddCellFloat (double value);

  /*! \brief add a cell with date and time data
   * \param  value         date and time value
   * \sa     NextRow()
   */
  void AddCellDateTime (time_t value);

  /*! \brief insertion operators
   * \sa     AddCellString()
   * \name   operator<<
   * \{
   */
  XLSXIOWriter& operator << (const char* value);
  XLSXIOWriter& operator << (const std::string& value);
  XLSXIOWriter& operator << (int64_t value);
  XLSXIOWriter& operator << (double value);
  //XLSXIOWriter& operator << (time_t value);
  /*! @} */

  /*! \brief mark the end of a row (next cell will start on a new row)
   * \sa     xlsxiowrite_next_row()
   * \sa     AddCellString()
   */
  void NextRow ();
};




inline XLSXIOWriter::XLSXIOWriter (const char* filename, const char* sheetname, size_t detectionrows)
{
  unlink(filename);
  handle = xlsxiowrite_open(filename, sheetname);
  xlsxiowrite_set_detection_rows(handle, detectionrows);
}

inline XLSXIOWriter::~XLSXIOWriter ()
{
  xlsxiowrite_close(handle);
}

inline void XLSXIOWriter::SetRowHeight (size_t height)
{
  xlsxiowrite_set_row_height(handle, height);
}

inline void XLSXIOWriter::AddColumn (const char* name, int width)
{
  xlsxiowrite_add_column(handle, name, width);
}

inline void XLSXIOWriter::AddCellString (const char* value)
{
  xlsxiowrite_add_cell_string(handle, value);
}

inline void XLSXIOWriter::AddCellInt (long long value)
{
  xlsxiowrite_add_cell_int(handle, value);
}

inline void XLSXIOWriter::AddCellFloat (double value)
{
  xlsxiowrite_add_cell_float(handle, value);
}

inline void XLSXIOWriter::AddCellDateTime (time_t value)
{
  xlsxiowrite_add_cell_datetime(handle, value);
}

inline XLSXIOWriter& XLSXIOWriter::operator << (const char* value)
{
  AddCellString(value); return *this;
}

inline XLSXIOWriter& XLSXIOWriter::operator << (const std::string& value)
{
  AddCellString(value.c_str());
  return *this;
}

inline XLSXIOWriter& XLSXIOWriter::operator << (int64_t value)
{
  AddCellInt(value);
  return *this;
}

inline XLSXIOWriter& XLSXIOWriter::operator << (double value)
{
  AddCellFloat(value);
  return *this;
}

/*
inline XLSXIOWriter& XLSXIOWriter::operator << (time_t value)
{
  AddCellDateTime(value);
  return *this;
}
*/

inline void XLSXIOWriter::NextRow ()
{
  xlsxiowrite_next_row(handle);
}



const char* filename = "example.xlsx";

int main (int argc, char* argv[])
{
  XLSXIOWriter* xlsxfile = new XLSXIOWriter(filename);
  xlsxfile->SetRowHeight(1);
  xlsxfile->AddColumn("Col1");
  xlsxfile->AddColumn("Col2");
  xlsxfile->AddColumn("Col3");
  xlsxfile->AddColumn("Col4");
  xlsxfile->AddColumn("Col5");
  xlsxfile->NextRow();
  int i;
  for (i = 0; i < 1000; i++) {
    *xlsxfile << "Test" << (char*)NULL << (int64_t)i;
    xlsxfile->AddCellDateTime(time(NULL));
    *xlsxfile << 3.1415926;
    xlsxfile->NextRow();
  }
  delete xlsxfile;
  return 0;
}
