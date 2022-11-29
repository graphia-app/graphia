/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include "glyphmap.h"

#include "shared/utils/container.h"
#include "app/preferences.h"

#include <QTextLayout>
#include <QPainter>
#include <QDebug>
#include <QGuiApplication>
#include <QDir>
#include <QPainterPath>

#include <memory>
#include <algorithm>

static const int TextureSize = 2048;

GlyphMap::GlyphMap(QString fontName) :
    _fontName(std::move(fontName))
{}

void GlyphMap::addText(const QString& text)
{
    const std::unique_lock<std::recursive_mutex> lock(_mutex);

    if(!text.isEmpty() && !u::contains(_results._layouts, text))
    {
        _results._layouts[text]._initialised = false;

        if(_updateTypeRequired < UpdateType::Layout)
            _updateTypeRequired = UpdateType::Layout;
    }
}

void GlyphMap::update()
{
    const std::unique_lock<std::recursive_mutex> lock(_mutex);

    if(!updateRequired())
        return;

    const int FontSize = 200;
    QFont font(_fontName, FontSize);

    layoutStrings(font);

    if(!stringsAreRenderable(font))
    {
        auto defaultFontName = QGuiApplication::font().family();

        qWarning() << "Font" << _fontName << "is not renderable; "
                      "falling back to" << defaultFontName;

        font = QFont(defaultFontName, FontSize);
        setFontName(defaultFontName);
        layoutStrings(font);
    }

    renderImages(font);

    _updateTypeRequired = UpdateType::None;
}

bool GlyphMap::updateRequired() const
{
    const std::unique_lock<std::recursive_mutex> lock(_mutex);

    return _updateTypeRequired != UpdateType::None;
}

const GlyphMap::Results& GlyphMap::results() const
{
    return _results;
}

const std::vector<QImage>& GlyphMap::images() const
{
    return _images;
}

void GlyphMap::setFontName(const QString& fontName)
{
    const std::unique_lock<std::recursive_mutex> lock(_mutex);

    if(_fontName != fontName && _updateTypeRequired < UpdateType::Images)
        _updateTypeRequired = UpdateType::Images;

    _fontName = fontName;
}

void GlyphMap::layoutStrings(const QFont& font)
{
    const QFontMetrics fontMetrics(font);

    const bool relayoutAllStrings = (_updateTypeRequired >= UpdateType::Images);

    if(relayoutAllStrings)
        _results._glyphs.clear();

    for(auto& textLayoutPair : _results._layouts)
    {
        if(!relayoutAllStrings && textLayoutPair.second._initialised)
            continue;

        const QString text = textLayoutPair.first;

        QTextLayout qTextLayout(text, font);
        QTextOption qTextOption;
        qTextOption.setWrapMode(QTextOption::NoWrap);
        qTextOption.setUseDesignMetrics(true);
        qTextLayout.setTextOption(qTextOption);
        qTextLayout.beginLayout();
        QTextLine line = qTextLayout.createLine();
        line.setNumColumns(static_cast<int>(text.size()));
        line.setPosition(QPointF(0.0f, 0.0f));
        qTextLayout.endLayout();

        QList<QGlyphRun> glyphRuns = line.glyphRuns(0, static_cast<int>(text.length()));

        // No need to continue if there are no glyphruns
        if(!glyphRuns.empty())
        {
            textLayoutPair.second._initialised = true;

            // Mistrust in Qt is good, right?
            Q_ASSERT(glyphRuns[0].glyphIndexes().size() == glyphRuns[0].positions().size());

            textLayoutPair.second._glyphs.clear();
            for(int i = 0; i < glyphRuns[0].glyphIndexes().size(); i++)
            {
                auto index = glyphRuns[0].glyphIndexes().at(i);
                auto advance = glyphRuns[0].positions().at(i).x() / TextureSize;
                textLayoutPair.second._glyphs.push_back({index, advance});
            }

            textLayoutPair.second._width = static_cast<float>(fontMetrics.boundingRect(text).width()) /
                    static_cast<float>(TextureSize);
            textLayoutPair.second._xHeight = static_cast<float>(fontMetrics.xHeight()) /
                    static_cast<float>(TextureSize);

            for(auto glyph : textLayoutPair.second._glyphs)
            {
                if(!u::contains(_results._glyphs, glyph._index))
                {
                    _results._glyphs[glyph._index] = {};

                    // New glyphs, so new images need to be generated
                    _updateTypeRequired = UpdateType::Images;
                }
            }
        }
    }
}

bool GlyphMap::stringsAreRenderable(const QFont &font) const
{
    auto rawFont = QRawFont::fromFont(font);

    // If there are zero glyphs whose paths are non-empty, this
    // is not something we want to use to render with
    return std::any_of(_results._glyphs.begin(), _results._glyphs.end(), [&](const auto& glyphPair)
    {
        auto glyph = glyphPair.first;
        auto path = rawFont.pathForGlyph(glyph);
        auto boundingRect = path.boundingRect();

        return !boundingRect.isEmpty();
    });
}

void GlyphMap::renderImages(const QFont &font)
{
    if(_results._glyphs.empty() || _updateTypeRequired < UpdateType::Images)
        return;

    auto rawFont = QRawFont::fromFont(font);

    _images.clear();

    // Render Glyphs
    float maxGlyphHeight = 0.0f;
    const float padding = std::max(static_cast<float>(QFontMetrics(font).height()) * 0.1f, 1.0f);
    float x = padding;
    float y = padding;
    int layer = 0;

    std::vector<QImage> debugImages;

    for(auto& glyphPair : _results._glyphs)
    {
        auto glyph = glyphPair.first;
        auto path = rawFont.pathForGlyph(glyph);
        auto boundingRect = path.boundingRect();
        auto glyphWidth = static_cast<float>(boundingRect.x() + boundingRect.width());
        auto glyphAscent = static_cast<float>(boundingRect.y());
        auto glyphHeight = static_cast<float>(boundingRect.height());
        auto right = x + glyphWidth + padding;

        auto paintGlyphTo = [&](QImage& image)
        {
            QPainter painter(&image);

            painter.setFont(font);
            painter.setPen(Qt::white);
            painter.fillPath(path.translated(x, y - glyphAscent), QBrush(Qt::white));
        };

        maxGlyphHeight = std::max(glyphHeight, maxGlyphHeight);

        if(right >= static_cast<float>(TextureSize))
        {
            // Move down onto a new row
            y += maxGlyphHeight + padding;
            x = padding;
            maxGlyphHeight = glyphHeight;

            const float bottom = y + maxGlyphHeight + padding;

            if(bottom >= static_cast<float>(TextureSize))
            {
                // Spill onto a new image
                layer++;
            }
        }

        if(static_cast<int>(_images.size()) == layer)
        {
            auto& image = _images.emplace_back(TextureSize, TextureSize, QImage::Format_ARGB32);
            image.fill(Qt::transparent);

            if(u::pref(QStringLiteral("debug/saveGlyphMaps")).toBool())
                debugImages.emplace_back(image.copy());

            // Reset paint coordinates
            x = padding;
            y = padding;
        };

        paintGlyphTo(_images.back());

        if(u::pref(QStringLiteral("debug/saveGlyphMaps")).toBool())
        {
            auto& debugImage = debugImages.back();
            paintGlyphTo(debugImage);

            QPainter debugPainter(&debugImage);

            auto xi = static_cast<int>(x);
            auto yi = static_cast<int>(y);
            auto wi = static_cast<int>(glyphWidth);
            auto hi = static_cast<int>(glyphHeight);
            auto ai = static_cast<int>(glyphAscent);

            debugPainter.setPen(Qt::red);
            debugPainter.drawLine(xi,      yi,      xi,      yi + hi);
            debugPainter.drawLine(xi,      yi,      xi + wi, yi);

            debugPainter.setPen(Qt::green);
            debugPainter.drawLine(xi,      yi - ai, xi + wi, yi - ai);

            debugPainter.setPen(Qt::yellow);
            debugPainter.drawLine(xi + wi, yi,      xi + wi, yi + hi);
            debugPainter.drawLine(xi,      yi + hi, xi + wi, yi + hi);
        }

        const float u = x / static_cast<float>(TextureSize);
        const float v = (y + glyphHeight) / static_cast<float>(TextureSize);
        const float w = glyphWidth / static_cast<float>(TextureSize);
        const float h = glyphHeight / static_cast<float>(TextureSize);
        const float a = glyphAscent / static_cast<float>(TextureSize);

        auto& textureGlyph = _results._glyphs[glyph];
        textureGlyph._layer = layer;
        textureGlyph._u = u;
        textureGlyph._v = v;
        textureGlyph._width = w;
        textureGlyph._height = h;
        textureGlyph._ascent = a;

        x += glyphWidth + padding;
    }

    // Save Glyphmap(s) for debug purposes if needed
    if(u::pref(QStringLiteral("debug/saveGlyphMaps")).toBool())
    {
        for(int i = 0; const auto& debugImage : debugImages)
            debugImage.save(QDir::currentPath() + "/GlyphMap" + QString::number(i++) + ".png");
    }
}
