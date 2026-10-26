/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef XLSXWRITER_H
#define XLSXWRITER_H

#include <QByteArray>
#include <QFile>
#include <QList>
#include <QString>
#include <QStringList>

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

// Writes the smallest XLSX file that Excel and xlsxio will read: a zip of
// uncompressed XML files, with a single sheet. Cells that are numbers are
// written as numbers, empty cells are left out, and anything else is
// written as a shared string.
namespace XlsxWriter
{
using Cells = QList<QStringList>;

inline uint32_t crc32(const QByteArray& data)
{
    static const auto table = []
    {
        std::array<uint32_t, 256> t{};
        for(uint32_t i = 0; i < 256; i++)
        {
            uint32_t c = i;
            for(int k = 0; k < 8; k++)
                c = (c & 1u) != 0 ? 0xEDB88320u ^ (c >> 1u) : c >> 1u;

            t.at(i) = c;
        }

        return t;
    }();

    uint32_t crc = 0xFFFFFFFFu;
    for(auto byte : data)
        crc = table.at((crc ^ static_cast<uint8_t>(byte)) & 0xFFu) ^ (crc >> 8u);

    return crc ^ 0xFFFFFFFFu;
}

inline void append16(QByteArray& data, uint32_t value)
{
    data.append(static_cast<char>(value & 0xFFu));
    data.append(static_cast<char>((value >> 8u) & 0xFFu));
}

inline void append32(QByteArray& data, uint32_t value)
{
    append16(data, value & 0xFFFFu);
    append16(data, value >> 16u);
}

// A zip of uncompressed files
inline QByteArray zip(const std::vector<std::pair<QByteArray, QByteArray>>& files)
{
    QByteArray archive;
    QByteArray centralDirectory;

    for(const auto& [name, content] : files)
    {
        const auto offset = static_cast<uint32_t>(archive.size());
        const auto crc = crc32(content);
        const auto size = static_cast<uint32_t>(content.size());
        const auto nameSize = static_cast<uint32_t>(name.size());

        append32(archive, 0x04034B50u); // Local file header
        append16(archive, 20);          // Version needed
        append16(archive, 0);           // Flags
        append16(archive, 0);           // Stored
        append16(archive, 0);           // Time
        append16(archive, 0x21);        // Date, 1980-01-01
        append32(archive, crc);
        append32(archive, size);
        append32(archive, size);
        append16(archive, nameSize);
        append16(archive, 0);           // Extra field length
        archive.append(name);
        archive.append(content);

        append32(centralDirectory, 0x02014B50u);
        append16(centralDirectory, 20); // Version made by
        append16(centralDirectory, 20); // Version needed
        append16(centralDirectory, 0);
        append16(centralDirectory, 0);
        append16(centralDirectory, 0);
        append16(centralDirectory, 0x21);
        append32(centralDirectory, crc);
        append32(centralDirectory, size);
        append32(centralDirectory, size);
        append16(centralDirectory, nameSize);
        append16(centralDirectory, 0);  // Extra field length
        append16(centralDirectory, 0);  // Comment length
        append16(centralDirectory, 0);  // Disk
        append16(centralDirectory, 0);  // Internal attributes
        append32(centralDirectory, 0);  // External attributes
        append32(centralDirectory, offset);
        centralDirectory.append(name);
    }

    const auto centralDirectoryOffset = static_cast<uint32_t>(archive.size());
    archive.append(centralDirectory);

    append32(archive, 0x06054B50u);    // End of central directory
    append16(archive, 0);
    append16(archive, 0);
    append16(archive, static_cast<uint32_t>(files.size()));
    append16(archive, static_cast<uint32_t>(files.size()));
    append32(archive, static_cast<uint32_t>(centralDirectory.size()));
    append32(archive, centralDirectoryOffset);
    append16(archive, 0);

    return archive;
}

inline QString columnName(qsizetype column)
{
    QString name;

    for(column++; column > 0; column = (column - 1) / 26)
        name.prepend(QChar(u'A' + static_cast<char16_t>((column - 1) % 26)));

    return name;
}

inline QByteArray xlsx(const Cells& cells)
{
    const QByteArray header = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)" "\n";
    const QByteArray main = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
    const QByteArray relationships = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
    const QByteArray packageRelationships = "http://schemas.openxmlformats.org/package/2006/relationships";

    QStringList sharedStrings;
    QByteArray sheetData;

    for(qsizetype row = 0; row < cells.size(); row++)
    {
        sheetData += "<row r=\"" + QByteArray::number(row + 1) + "\">";

        for(qsizetype column = 0; column < cells.at(row).size(); column++)
        {
            const auto& value = cells.at(row).at(column);
            if(value.isEmpty())
                continue;

            const auto reference = (columnName(column) + QString::number(row + 1)).toUtf8();

            bool isNumber = false;
            value.toDouble(&isNumber); // clazy:exclude=unused-result-check

            if(isNumber)
                sheetData += "<c r=\"" + reference + "\"><v>" + value.toUtf8() + "</v></c>";
            else
            {
                auto index = sharedStrings.indexOf(value);
                if(index < 0)
                {
                    index = sharedStrings.size();
                    sharedStrings.append(value);
                }

                sheetData += "<c r=\"" + reference + "\" t=\"s\"><v>" + QByteArray::number(index) + "</v></c>";
            }
        }

        sheetData += "</row>";
    }

    QByteArray strings;
    for(const auto& string : sharedStrings)
        strings += "<si><t>" + string.toHtmlEscaped().toUtf8() + "</t></si>";

    return zip(
    {
        {"[Content_Types].xml", header +
            R"(<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">)"
            R"(<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>)"
            R"(<Default Extension="xml" ContentType="application/xml"/>)"
            R"(<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>)"
            R"(<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>)"
            R"(<Override PartName="/xl/sharedStrings.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"/>)"
            "</Types>"},
        {"_rels/.rels", header +
            "<Relationships xmlns=\"" + packageRelationships + "\">"
            "<Relationship Id=\"rId1\" Type=\"" + relationships + "/officeDocument\" Target=\"xl/workbook.xml\"/>"
            "</Relationships>"},
        {"xl/workbook.xml", header +
            "<workbook xmlns=\"" + main + "\" xmlns:r=\"" + relationships + "\">"
            "<sheets><sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"rId1\"/></sheets>"
            "</workbook>"},
        {"xl/_rels/workbook.xml.rels", header +
            "<Relationships xmlns=\"" + packageRelationships + "\">"
            "<Relationship Id=\"rId1\" Type=\"" + relationships + "/worksheet\" Target=\"worksheets/sheet1.xml\"/>"
            "<Relationship Id=\"rId2\" Type=\"" + relationships + "/sharedStrings\" Target=\"sharedStrings.xml\"/>"
            "</Relationships>"},
        {"xl/worksheets/sheet1.xml", header +
            "<worksheet xmlns=\"" + main + "\"><sheetData>" + sheetData + "</sheetData></worksheet>"},
        {"xl/sharedStrings.xml", header +
            "<sst xmlns=\"" + main + "\" count=\"" + QByteArray::number(sharedStrings.size()) +
            "\" uniqueCount=\"" + QByteArray::number(sharedStrings.size()) + "\">" + strings + "</sst>"},
    });
}

inline bool write(const QString& fileName, const Cells& cells)
{
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    const auto data = xlsx(cells);
    return file.write(data) == data.size();
}
} // namespace XlsxWriter

#endif // XLSXWRITER_H
