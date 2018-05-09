#include "glyphmap.h"

#include "shared/utils/container.h"
#include "shared/utils/preferences.h"

#include <QTextLayout>
#include <QPainter>
#include <QDebug>
#include <QGuiApplication>
#include <QDir>

#include <memory>

GlyphMap::GlyphMap(QString fontName) :
    _fontName(std::move(fontName))
{}

void GlyphMap::addText(const QString& text)
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    if(!text.isEmpty() && !u::contains(_results._layouts, text))
    {
        _results._layouts[text]._initialised = false;

        if(_updateTypeRequired < UpdateType::Layout)
            _updateTypeRequired = UpdateType::Layout;
    }
}

void GlyphMap::update()
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    if(!updateRequired())
        return;

    QFont font(_fontName, _fontSize);

    layoutStrings(font);

    if(!stringsAreRenderable(font))
    {
        auto defaultFontName = QGuiApplication::font().family();

        qWarning() << "Font" << _fontName << "is not renderable; "
                      "falling back to" << defaultFontName;

        font = QFont(defaultFontName, _fontSize);
        setFontName(defaultFontName);
        layoutStrings(font);
    }

    renderImages(font);

    _updateTypeRequired = UpdateType::None;
}

bool GlyphMap::updateRequired() const
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);

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

void GlyphMap::setTextureSize(int textureSize)
{
    _textureSize = textureSize;
}

void GlyphMap::setFontName(const QString& fontName)
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    if(_fontName != fontName && _updateTypeRequired < UpdateType::Images)
        _updateTypeRequired = UpdateType::Images;

    _fontName = fontName;
}

void GlyphMap::layoutStrings(const QFont& font)
{
    QFontMetrics fontMetrics(font);

    bool relayoutAllStrings = (_updateTypeRequired >= UpdateType::Images);

    if(relayoutAllStrings)
        _results._glyphs.clear();

    for(auto& textLayoutPair : _results._layouts)
    {
        if(!relayoutAllStrings && textLayoutPair.second._initialised)
            continue;

        QString text = textLayoutPair.first;

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

        QList<QGlyphRun> glyphRuns = line.glyphRuns(0, text.length());

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
                auto advance = glyphRuns[0].positions().at(i).x() / _textureSize;
                textLayoutPair.second._glyphs.push_back({index, advance});
            }

            textLayoutPair.second._width = static_cast<float>(fontMetrics.boundingRect(text).width()) /
                    static_cast<float>(_textureSize);
            textLayoutPair.second._xHeight = static_cast<float>(fontMetrics.xHeight()) /
                    static_cast<float>(_textureSize);

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
    for(auto& glyphPair : _results._glyphs)
    {
        auto glyph = glyphPair.first;
        auto path = rawFont.pathForGlyph(glyph);
        auto boundingRect = path.boundingRect();
        if(!boundingRect.isEmpty())
            return true;
    }

    return false;
}

void GlyphMap::renderImages(const QFont &font)
{
    if(_results._glyphs.empty() || _updateTypeRequired < UpdateType::Images)
        return;

    auto rawFont = QRawFont::fromFont(font);

    _images.clear();

    // Render Glyphs
    float maxGlyphHeight = 0.0f;
    float padding = std::max(QFontMetrics(font).height() * 0.1f, 1.0f);
    float x = padding;
    float y = padding;
    int layer = 0;

    std::unique_ptr<QPainter> textPainter;

    for(auto& glyphPair : _results._glyphs)
    {
        auto glyph = glyphPair.first;
        auto path = rawFont.pathForGlyph(glyph);
        auto boundingRect = path.boundingRect();
        float glyphWidth = boundingRect.x() + boundingRect.width();
        float glyphAscent = boundingRect.y();
        float glyphHeight = boundingRect.height();
        float right = x + glyphWidth + padding;

        maxGlyphHeight = std::max(glyphHeight, maxGlyphHeight);

        if(right >= _textureSize)
        {
            // Move down onto a new row
            y += maxGlyphHeight + padding;
            x = padding;
            maxGlyphHeight = glyphHeight;

            float bottom = y + maxGlyphHeight + padding;

            if(bottom >= _textureSize)
            {
                // Spill onto a new image
                textPainter = nullptr;
                layer++;
            }
        }

        if(textPainter == nullptr)
        {
            _images.emplace_back(_textureSize, _textureSize, QImage::Format_ARGB32);
            auto& image = _images.back();

            image.fill(Qt::transparent);

            textPainter = std::make_unique<QPainter>(&image);
            textPainter->setFont(font);
            textPainter->setPen(Qt::white);

            // Reset paint coordinates
            x = padding;
            y = padding;
        }

        path.translate(x, y - glyphAscent);
        textPainter->fillPath(path, QBrush(Qt::white));

        if(u::pref("debug/saveGlyphMaps").toBool())
        {
            textPainter->setPen(Qt::red);
            textPainter->drawLine(x, y, x, y + glyphHeight);
            textPainter->drawLine(x, y, x + glyphWidth, y);

            textPainter->setPen(Qt::green);
            textPainter->drawLine(x, y - glyphAscent, x + glyphWidth, y - glyphAscent);

            textPainter->setPen(Qt::yellow);
            textPainter->drawLine(x + glyphWidth, y, x + glyphWidth, y + glyphHeight);
            textPainter->drawLine(x, y + glyphHeight, x + glyphWidth, y + glyphHeight);
        }

        auto& image = _images.back();

        float u = static_cast<float>(x) / image.width();
        float v = static_cast<float>(y + glyphHeight) / image.height();
        float w = static_cast<float>(glyphWidth) / image.width();
        float h = static_cast<float>(glyphHeight) / image.height();
        float a = static_cast<float>(glyphAscent) / image.height();

        auto& textureGlyph = _results._glyphs[glyph];
        textureGlyph._layer = layer;
        textureGlyph._u = u;
        textureGlyph._v = v;
        textureGlyph._width = w;
        textureGlyph._height = h;
        textureGlyph._ascent = a;

        x += glyphWidth + padding;
    }

    // Save Glyphmap for debug purposes if needed
    if(u::pref("debug/saveGlyphMaps").toBool())
    {
        for(int i = 0; i < static_cast<int>(_images.size()); i++)
            _images[i].save(QDir::currentPath() + "/GlyphMap" + QString::number(i) + ".png");
    }
}
