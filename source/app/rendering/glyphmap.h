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

#ifndef GLYPHMAP_H
#define GLYPHMAP_H

#include <QVector2D>
#include <QString>
#include <QImage>
#include <QFont>
#include <QGlyphRun>
#include <QFontMetrics>

#include <map>
#include <mutex>

class GlyphMap
{
public:
    struct Results
    {
        struct StringLayout
        {
            bool _initialised = false;

            struct Glyph
            {
                quint32 _index;
                double _advance;
            };

            std::vector<Glyph> _glyphs;
            float _width = -1.0f;
            float _xHeight = -1.0f;
        };

        struct TextureGlyph
        {
            int _layer = -1;

            float _u = -1.0f;
            float _v = -1.0f;
            float _width = -1.0f;
            float _height = -1.0f;

            float _ascent = -1.0f;
        };

        std::map<QString, StringLayout> _layouts;
        std::map<quint32, TextureGlyph> _glyphs;
    };

private:
    std::vector<QImage> _images;
    Results _results;
    QString _fontName;

    enum class UpdateType
    {
        None,
        Layout,
        Images
    };

    UpdateType _updateTypeRequired = UpdateType::Images;

    mutable std::recursive_mutex _mutex;

public:
    explicit GlyphMap(QString fontName);

    void addText(const QString& text);
    void update();
    bool updateRequired() const;
    const Results& results() const;

    const std::vector<QImage>& images() const;

    void setFontName(const QString& fontName);

    std::recursive_mutex& mutex() { return _mutex; }

private:
    void layoutStrings(const QFont& font);
    bool stringsAreRenderable(const QFont& font) const;
    void renderImages(const QFont& font);
};

#endif // GLYPHMAP_H
