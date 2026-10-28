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

#include "app/rendering/openglfunctions.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLVertexArrayObject>
#include <QTest>

#include <thread>
#include <utility>

class TestOpenGL : public QObject
{
    Q_OBJECT

private slots:
    void available()
    {
        QOpenGLContext context;
        context.setFormat(OpenGLFunctions::minimumFormat());
        QVERIFY(context.create());

        QOffscreenSurface surface;
        surface.setFormat(context.format());
        surface.create();
        QVERIFY(context.makeCurrent(&surface));

        OpenGLFunctions f;
        QVERIFY(f.initializeOpenGLFunctions());

        // The context's format can be the one requested rather than the one obtained
        GLint major = 0;
        GLint minor = 0;
        f.glGetIntegerv(GL_MAJOR_VERSION, &major);
        f.glGetIntegerv(GL_MINOR_VERSION, &minor);
        QVERIFY(std::pair(major, minor) >= OpenGLFunctions::minimumFormat().version());

        context.doneCurrent();
    }

    // The surface is made on the GUI thread, since on some platforms it's a
    // window, and used by a context on a thread of its own
    void currentOnAnotherThread()
    {
        QOffscreenSurface surface;
        surface.setFormat(OpenGLFunctions::minimumFormat());
        surface.create();
        QVERIFY(surface.isValid());

        bool created = false;
        bool current = false;
        bool vertexArrayCreated = false;

        std::thread thread([&]
        {
            QOpenGLContext context;
            context.setFormat(OpenGLFunctions::minimumFormat());
            created = context.create();
            if(!created)
                return;

            current = context.makeCurrent(&surface);
            if(!current)
                return;

            // A core profile context draws nothing without a vertex array bound
            QOpenGLVertexArrayObject vertexArray;
            vertexArrayCreated = vertexArray.create();
            vertexArray.bind();

            vertexArray.destroy();
            context.doneCurrent();
        });
        thread.join();

        QVERIFY(created);
        QVERIFY(current);
        QVERIFY(vertexArrayCreated);
    }
};

QTEST_MAIN(TestOpenGL)
#include "tst_opengl.moc"
