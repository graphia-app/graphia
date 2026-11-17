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

#include "app/rendering/glyphmap.h"
#include "app/rendering/graphrenderercore.h"
#include "app/rendering/openglfunctions.h"
#include "app/rendering/shadertools.h"

#include "tests/common/testenvironment.h"

#include <QGuiApplication>
#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QSize>
#include <QString>
#include <QTest>

#include <memory>
#include <vector>

using namespace Qt::Literals::StringLiterals;

// Draws the SDF of a glyph map, and the reference that it must match
class SdfRenderer : public GraphRendererCore
{
public:
    std::vector<QImage> sdf(const GlyphMap& glyphMap)
    {
        GLuint texture = 0;
        glGenTextures(1, &texture);

        const auto size = renderSdfTexture(glyphMap, texture);

        GLuint fbo = 0;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        std::vector<QImage> layers;
        for(size_t layer = 0; layer < glyphMap.images().size(); layer++)
        {
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, static_cast<GLint>(layer));
            layers.push_back(read(size));
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &texture);

        return layers;
    }

    std::vector<QImage> reference(const GlyphMap& glyphMap, QSize size)
    {
        QOpenGLShaderProgram program;
        ShaderTools::loadShaderProgram(program, u":/shaders/fullscreen.vert"_s, u":/rendering/sdfreference.frag"_s);

        QOpenGLVertexArrayObject vao;
        vao.create();
        vao.bind();

        GLuint source = 0;
        glGenTextures(1, &source);
        glBindTexture(GL_TEXTURE_2D, source);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        GLuint target = 0;
        glGenTextures(1, &target);
        glBindTexture(GL_TEXTURE_2D, target);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.width(), size.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        GLuint fbo = 0;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target, 0);

        glViewport(0, 0, size.width(), size.height());
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        program.bind();
        program.setUniformValue("tex", 0);

        std::vector<QImage> layers;
        layers.reserve(glyphMap.images().size());
        for(const auto& glyphImage : glyphMap.images())
        {
            const auto image = glyphImage.flipped().convertToFormat(QImage::Format_RGBA8888);

            program.setUniformValue("texSize", QPoint(image.width(), image.height()));
            program.setUniformValue("scaleFactor", static_cast<GLfloat>(image.width()) / static_cast<GLfloat>(size.width()));

            glBindTexture(GL_TEXTURE_2D, source);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(),
                0, GL_RGBA, GL_UNSIGNED_BYTE, image.constBits());

            glDrawArrays(GL_TRIANGLES, 0, 3);
            layers.push_back(read(size));
        }

        program.release();
        vao.release();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &target);
        glDeleteTextures(1, &source);

        return layers;
    }

    GLuint sdfTexture() const override { return 0; }

private:
    QImage read(QSize size)
    {
        QImage image(size, QImage::Format_RGBA8888);
        glReadPixels(0, 0, size.width(), size.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
        return image;
    }
};

class TestSdf : public QObject
{
    Q_OBJECT

    QOpenGLContext _context;
    QOffscreenSurface _surface;

    // Enough glyphs to fill more than one layer
    static std::unique_ptr<GlyphMap> glyphMap()
    {
        auto map = std::make_unique<GlyphMap>(QGuiApplication::font().family());

        for(char16_t c = u'!'; c <= u'~'; c++)
            map->addText(QString(QChar(c)));

        for(char16_t c = u'À'; c <= u'ÿ'; c++)
            map->addText(QString(QChar(c)));

        map->update();
        return map;
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();

        _context.setFormat(OpenGLFunctions::minimumFormat());
        QVERIFY(_context.create());

        _surface.setFormat(_context.format());
        _surface.create();
        QVERIFY(_context.makeCurrent(&_surface));
    }

    void cleanupTestCase() { _context.doneCurrent(); }

    void matchesTheSearchOfEveryTexel()
    {
        const auto map = glyphMap();
        QVERIFY(map->images().size() > 1);

        SdfRenderer renderer;
        const auto sdf = renderer.sdf(*map);
        const auto reference = renderer.reference(*map, sdf.front().size());

        // Another renderer reuses the SDF that the first one drew
        const auto reused = SdfRenderer().sdf(*map);

        QCOMPARE(sdf.size(), reference.size());
        QCOMPARE(reused.size(), reference.size());
        for(size_t layer = 0; layer < sdf.size(); layer++)
        {
            QCOMPARE(sdf.at(layer), reference.at(layer));
            QCOMPARE(reused.at(layer), reference.at(layer));
        }
    }
};

QTEST_MAIN(TestSdf)
#include "tst_sdf.moc"
