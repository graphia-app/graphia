#include "glyphmap.h"

#include "shared/utils/utils.h"
#include "shared/utils/preferences.h"

#include <QTextLayout>
#include <QPainter>
#include <QDebug>
#include <QGuiApplication>
#include <QDir>

#include <memory>

GlyphMap::GlyphMap(const QString& fontName) :
    _fontName(fontName)
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

void GlyphMap::setMaxTextureSize(int maxTextureSize)
{
    _maxTextureSize = maxTextureSize;
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
        if(glyphRuns.size() > 0)
        {
            textLayoutPair.second._initialised = true;

            // Mistrust in Qt is good, right?
            Q_ASSERT(glyphRuns[0].glyphIndexes().size() == glyphRuns[0].positions().size());

            textLayoutPair.second._glyphs.clear();
            for(int i = 0; i < glyphRuns[0].glyphIndexes().size(); i++)
            {
                auto index = glyphRuns[0].glyphIndexes().at(i);
                auto advance = glyphRuns[0].positions().at(i).x() / _maxTextureSize;
                textLayoutPair.second._glyphs.push_back({index, advance});
            }

            textLayoutPair.second._width = static_cast<float>(fontMetrics.width(text)) /
                    static_cast<float>(_maxTextureSize);

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

bool GlyphMap::stringsAreRenderable(const QFont &font)
{
    auto rawFont = QRawFont::fromFont(font);

    // If there are zero glyphs whose paths are non-empty paths, this
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

    QFontMetrics fontMetrics(font);
    auto rawFont = QRawFont::fromFont(font);

    int imageCount = 1;

    int fontHeight = fontMetrics.height();
    int padding = std::max(fontHeight / 10, 1);

    _images.clear();

    // Calculate Image Width
    int imageWidth = _maxTextureSize;
    int imageHeight = fontHeight + (2 * padding);
    int x = padding;

    for(auto& glyphPair : _results._glyphs)
    {
        auto glyph = glyphPair.first;

        auto path = rawFont.pathForGlyph(glyph);
        x += path.boundingRect().width() * 2 + padding;
        if(x >= _maxTextureSize)
        {
            x = path.boundingRect().width() * 2;
            imageHeight += fontHeight + padding;
            if(imageHeight >= _maxTextureSize)
            {
                imageCount++;
                imageHeight = fontHeight + (2 * padding);
            }
        }
    }

    if(imageCount > 1)
        imageHeight = _maxTextureSize;

    x = padding;

    // Generate Blank Images
    for(int i = 0; i < imageCount; i++)
    {
       _images.emplace_back(imageWidth, imageHeight, QImage::Format_ARGB32);
       _images.back().fill(Qt::transparent);
    }

    // Render Glyphs
    int glyphHeight = fontMetrics.height();
    int y = padding, layer = 0;

    std::unique_ptr<QPainter> textPainter = std::make_unique<QPainter>(&_images[0]);
    textPainter->setFont(font);
    textPainter->setPen(Qt::GlobalColor::white);
    auto fillBrush = QBrush(Qt::GlobalColor::white);

    for(auto& glyphPair : _results._glyphs)
    {
        auto glyph = glyphPair.first;

        auto path = rawFont.pathForGlyph(glyph);
        int glyphWidth = path.boundingRect().width() * 2;

        // If X exceeds maxTexture Size
        if(x + glyphWidth + padding >= _maxTextureSize)
        {
            // New row layer if exceed x size
            y += glyphHeight + padding;
            x = padding;
            // If Y exceeds maxTexture Size
            if(y + glyphHeight + padding >= _maxTextureSize)
            {
                // New image layer if exceed rowcount
                layer++;
                x = padding;
                y = padding;

                // Remake the painter with the new image++
                textPainter = std::make_unique<QPainter>(&_images[layer]);
                textPainter->setFont(font);
                textPainter->setPen(Qt::white);
            }
        }

        path.translate(x, y + fontMetrics.ascent());
        textPainter->fillPath(path, fillBrush);

        float u = static_cast<float>(x) / imageWidth;
        float v = static_cast<float>(y + glyphHeight) / imageHeight;
        float w = static_cast<float>(glyphWidth) / imageWidth;
        float h = static_cast<float>(glyphHeight) / imageHeight;

        auto& textureGlyph = _results._glyphs[glyph];
        textureGlyph._layer = layer;
        textureGlyph._u = u;
        textureGlyph._v = v;
        textureGlyph._width = w;
        textureGlyph._height = h;

        x += glyphWidth + padding;
    }

    // Save Glyphmap for debug purposes if needed
    if(u::prefExists("debug/saveGlyphMaps") &&
       u::pref("debug/saveGlyphMaps").toBool())
    {
        for(int i = 0; i < static_cast<int>(_images.size()); i++)
            _images[i].save(QDir::currentPath() + "/GlyphMap" + QString::number(i) + ".png");
    }
}
