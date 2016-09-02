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
        };

        struct TextureGlyph
        {
            float _u = -1.0f;
            float _v = -1.0f;
            float _width = -1.0f;
            float _height = -1.0f;
        };

        std::map<QString, StringLayout> _layouts;
        std::map<quint32, TextureGlyph> _glyphs;
    };

private:
    std::vector<QImage> _images;

    Results _results;

    QString _fontName;
    int _maxTextureSize = 2048;
    int _fontSize = 200;

    enum class UpdateType
    {
        None,
        Layout,
        Images
    };

    UpdateType _updateTypeRequired = UpdateType::Images;

    mutable std::recursive_mutex _mutex;

public:
    GlyphMap(const QString& fontName);

    void addText(const QString& text);
    void update();
    bool updateRequired() const;
    const Results& results() const;

    const std::vector<QImage>& images() const;

    void setMaxTextureSize(int maxTextureSize);

    void setFontName(const QString& fontName);

    std::recursive_mutex& mutex() { return _mutex; }

private:
    void layoutStrings();
    void renderImages();
};

#endif // GLYPHMAP_H
