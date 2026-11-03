#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string>
#include "xlsxio_read.h"

/*! \class XLSXIOReader
 *  \brief class for reading data from an .xlsx file
 *\details C++ wrapper for xlsxioread_ functions.
 */
class XLSXIOReader
{
 private:
  xlsxioreader handle;
 public:

  /*! \brief XLSXIOReader constructor, opens .xlsx file
   * \param  filename      path of .xlsx file to open
   * \sa     xlsxioread_open()
   */
  XLSXIOReader (const char* filename);

  /*! \brief XLSXIOReader destructor, closes .xlsx file
   * \sa     xlsxioread_close()
   */
  ~XLSXIOReader ();

  /*! \brief opens
   * \param  sheetname     worksheet name (NULL for first sheet)
   * \param  flags         XLSXIOREAD_SKIP_ flag(s) to determine how data is processed
   * \return XLSXIOReaderSheet object or NULL in case of error
   * \sa     xlsxioread_sheet_open()
   */
  class XLSXIOReaderSheet* OpenSheet (const char* sheetname, unsigned int flags);
};



/*! \class XLSXIOReaderSheet
 *  \brief class for reading data from a sheet in an .xlsx file
 *\details C++ wrapper for xlsxioread_sheet_ functions.
 */
class XLSXIOReaderSheet
{
  friend class XLSXIOReader;
 private:
  xlsxioreadersheet sheethandle;
  XLSXIOReaderSheet (xlsxioreadersheet sheet);
  XLSXIOReaderSheet (xlsxioreader xlsxhandle, const char* sheetname, unsigned int flags);
 public:

  /*! \brief XLSXIOReaderSheet, closes sheet
   * \sa     xlsxioread_sheet_close()
   */
  ~XLSXIOReaderSheet ();

  /*! \brief start reading the next row of data
   * \sa     xlsxioread_sheet_next_row()
   * \sa     GetNextCell()
   */
  bool GetNextRow ();

  /*! \brief read the next column cell
   * \return value (caller must free the result) or NULL if no more cells are available in the current row
   * \sa     xlsxioread_sheet_next_cell()
   * \sa     GetNextRow()
   */
  char* GetNextCell ();

  /*! \brief read the next column cell as a dynamically allocated string value
   * \param  value         reference where value will be stored (caller must free the result)
   * \return true if cell data was available, otherwise false
   * \sa     xlsxioread_sheet_next_cell_string()
   * \sa     GetNextRow()
   */
  bool GetNextCellString (char*& value);

  /*! \brief read the next column cell as a string value
   * \param  value         reference where value will be stored
   * \return true if cell data was available, otherwise false
   * \sa     xlsxioread_sheet_next_cell_string()
   * \sa     GetNextRow()
   */
  bool GetNextCellString (std::string& value);

  /*! \brief read the next column cell as an integer value
   * \param  value         reference where value will be stored
   * \return true if cell data was available, otherwise false
   * \sa     xlsxioread_sheet_next_cell_int()
   * \sa     GetNextRow()
   */
  bool GetNextCellInt (int64_t& value);

  /*! \brief read the next column cell as a floating point value
   * \param  value         reference where value will be stored
   * \return true if cell data was available, otherwise false
   * \sa     xlsxioread_sheet_next_cell_float()
   * \sa     GetNextRow()
   */
  bool GetNextCellFloat (double& value);

  /*! \brief read the next column cell as a date/time value
   * \param  value         reference where value will be stored
   * \return true if cell data was available, otherwise false
   * \sa     xlsxioread_sheet_next_cell_datetime()
   * \sa     GetNextRow()
   */
  bool GetNextCellDateTime (time_t& value);

  /*! \brief extraction operators
   * \sa     GetNextCellString()
   * \name   operator>>
   * \{
   */
  XLSXIOReaderSheet& operator >> (char*& value);
  XLSXIOReaderSheet& operator >> (std::string& value);
  XLSXIOReaderSheet& operator >> (int64_t& value);
  XLSXIOReaderSheet& operator >> (double& value);
  //inline XLSXIOReaderSheet& operator >> (time_t& value);
  /*! @} */
};



inline XLSXIOReader::XLSXIOReader (const char* filename)
{
  handle = xlsxioread_open(filename);
}

inline XLSXIOReader::~XLSXIOReader ()
{
  xlsxioread_close(handle);
}

inline class XLSXIOReaderSheet* XLSXIOReader::OpenSheet (const char* sheetname, unsigned int flags)
{
  xlsxioreadersheet sheethandle;
  if ((sheethandle = xlsxioread_sheet_open(handle, sheetname, flags)) == NULL)
    return NULL;
  return new XLSXIOReaderSheet(sheethandle);
}



inline XLSXIOReaderSheet::XLSXIOReaderSheet (xlsxioreadersheet sheet)
: sheethandle(sheet)
{
}

inline XLSXIOReaderSheet::~XLSXIOReaderSheet ()
{
  xlsxioread_sheet_close(sheethandle);
}

inline bool XLSXIOReaderSheet::GetNextRow ()
{
  return (xlsxioread_sheet_next_row(sheethandle) != 0);
}

inline char* XLSXIOReaderSheet::GetNextCell ()
{
  return xlsxioread_sheet_next_cell(sheethandle);
}

inline bool XLSXIOReaderSheet::GetNextCellString (char*& value)
{
  if (!xlsxioread_sheet_next_cell_string(sheethandle, &value)) {
    value = NULL;
    return false;
  }
  return true;
}

inline bool XLSXIOReaderSheet::GetNextCellString (std::string& value)
{
  char* result;
  if (!xlsxioread_sheet_next_cell_string(sheethandle, &result)) {
    value.clear();
    return false;
  }
  value.assign(result);
  free(result);
  return true;
}

inline bool XLSXIOReaderSheet::GetNextCellInt (int64_t& value)
{
  if (!xlsxioread_sheet_next_cell_int(sheethandle, &value)) {
    value = 0;
    return false;
  }
  return true;
}

inline bool XLSXIOReaderSheet::GetNextCellFloat (double& value)
{
  if (!xlsxioread_sheet_next_cell_float(sheethandle, &value)) {
    value = 0;
    return false;
  }
  return true;
}

inline bool XLSXIOReaderSheet::GetNextCellDateTime (time_t& value)
{
  if (!xlsxioread_sheet_next_cell_datetime(sheethandle, &value)) {
    value = 0;
    return false;
  }
  return true;
}

XLSXIOReaderSheet& XLSXIOReaderSheet::operator >> (char*& value)
{
  GetNextCellString(value);
  return *this;
}

XLSXIOReaderSheet& XLSXIOReaderSheet::operator >> (std::string& value)
{
  GetNextCellString(value);
  return *this;
}

XLSXIOReaderSheet& XLSXIOReaderSheet::operator >> (int64_t& value)
{
  GetNextCellInt(value);
  return *this;
}

XLSXIOReaderSheet& XLSXIOReaderSheet::operator >> (double& value)
{
  GetNextCellFloat(value);
  return *this;
}

/*
XLSXIOReaderSheet& XLSXIOReaderSheet::operator >> (time_t& value)
{
  GetNextCellDateTime(value);
  return *this;
}
*/



const char* filename = "example.xlsx";

int main (int argc, char* argv[])
{
  XLSXIOReader* xlsxfile = new XLSXIOReader(filename);
  XLSXIOReaderSheet* xlsxsheet = xlsxfile->OpenSheet(NULL, XLSXIOREAD_SKIP_EMPTY_ROWS);
  if (xlsxsheet) {
    std::string value;
    while (xlsxsheet->GetNextRow()) {
/*
      while (xlsxsheet->GetNextCellString(value)) {
        printf("%s\t", value.c_str());
      }
*/
      std::string s;
      *xlsxsheet >> s;
      printf("%s\t", s.c_str());
      char* n;
      *xlsxsheet >> n;
      printf("%s\t", n);
      free(n);
      int64_t i;
      *xlsxsheet >> i;
      //printf("%" PRIi64 "\t", i);
      printf("%li\t", (long)i);
      *xlsxsheet >> i;
      //printf("%" PRIi64 "\t", i);
      printf("%li\t", (long)i);
      double d;
      *xlsxsheet >> d;
      printf("%.6G\t", d);
      printf("\n");
    }
    delete xlsxsheet;
  }
  delete xlsxfile;
  return 0;
}
