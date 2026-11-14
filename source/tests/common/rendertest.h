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

#ifndef RENDERTEST_H
#define RENDERTEST_H

#include "tests/common/documenttest.h"

#include "app/rendering/openglfunctions.h"

#include "shared/utils/json.h"
#include "shared/utils/string.h"

#include <QByteArray>
#include <QColor>
#include <QCoreApplication>
#include <QImage>
#include <QMutex>
#include <QMutexLocker>
#include <QPointF>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QVector3D>
#include <QtLogging>

#include <algorithm>
#include <map>
#include <memory>
#include <utility>

// A DocumentTest whose GraphDisplay is shown in a window, so that the real renderer draws it
class RenderTest : public DocumentTest // clazy:exclude=missing-qobject-macro
{
public:
    // QTEST_MAIN calls this before it makes the application, as main() sets these up
    static void initMain()
    {
        QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
        QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
        QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
        OpenGLFunctions::requestMinimumFormat();

        // A debug context, whose high severity messages are logged
        qputenv("QSG_OPENGL_DEBUG", "1");
        qputenv("OPENGL_DEBUG", "1");
    }

protected:
    static constexpr int Width = 400;
    static constexpr int Height = 300;

    std::unique_ptr<QQuickWindow> _window;

    // The pixels showing one colour in a grab, and where they are
    struct Patch
    {
        int _pixels = 0;
        QPointF _centre;
    };

    bool open(const QByteArray& gml)
    {
        if(!load(write(u"graph.gml"_s, gml), u"GML"_s, u"Generic"_s, {}))
            return false;

        _window = std::make_unique<QQuickWindow>();
        _window->resize(Width, Height);
        _graphDisplay->setParentItem(_window->contentItem());
        _graphDisplay->setSize({Width, Height});

        _window->show();
        if(!QTest::qWaitForWindowExposed(_window.get()))
            return false;

        if(!QTest::qWaitFor([this] { return _graphDisplay->property("initialised").toBool(); }, Timeout))
            return false;

        return waitForCommands();
    }

    // The renderer reads the document on the render thread until its window is gone
    void close()
    {
        _window.reset();
        closeDocument();
    }

    // Pauses the layout and puts each named node where it says
    bool pin(const std::map<QString, QVector3D>& positions)
    {
        _document->setUserLayoutPaused(true);
        if(!waitForCommands())
            return false;

        auto array = json::array();
        for(const auto& [name, position] : positions)
        {
            array.push_back({{"id", static_cast<int>(nodeId(name))},
                {"position", {position.x(), position.y(), position.z()}}});
        }

        const auto url = write(u"positions.json"_s, QByteArray::fromStdString(array.dump()));
        if(!execute([&] { _document->loadNodePositionsFromFile(url); }))
            return false;

        // Until it is reset, the view frames the nodes where the layout had them
        _document->resetView();
        return !grab().isNull();
    }

    // Three nodes, a, b and c, with no edge between a and c, pinned left, right and above
    bool openTriangle()
    {
        const QByteArray triangle = R"(graph
[
    node [ id 0 label "a" ]
    node [ id 1 label "b" ]
    node [ id 2 label "c" ]
    edge [ source 0 target 1 ]
    edge [ source 1 target 2 ]
]
)";

        return open(triangle) &&
            pin({{u"a"_s, {-10.0f, -6.0f, 0.0f}}, {u"b"_s, {10.0f, -6.0f, 0.0f}}, {u"c"_s, {0.0f, 10.0f, 0.0f}}});
    }

    bool colourTriangle()
    {
        return colour({{u"a"_s, Qt::red}, {u"b"_s, Qt::green}, {u"c"_s, Qt::yellow}});
    }

    // Does something that changes the visuals, then waits for the change to reach the graph model,
    // which happens off the main thread and is not a command
    template<typename Fn>
    bool changeVisuals(const Fn& fn)
    {
        auto changed = std::make_shared<bool>(false);
        const auto connection = QObject::connect(graphModel(), &GraphModel::visualsChanged,
            _document.get(), [changed] { *changed = true; });

        fn();
        const bool result = QTest::qWaitFor([&] { return *changed; }, Timeout) && waitForCommands();

        QObject::disconnect(connection);
        return result;
    }

    // Gives each named node a colour of its own, so that it can be found in a grab
    bool colour(const std::map<QString, QColor>& colours)
    {
        auto fixedColors = json::object();
        for(const auto& [name, color] : colours)
            fixedColors[name.toStdString()] = color.name().toStdString();

        const auto palette = QString::fromStdString(json{{"fixedColors", fixedColors}}.dump());
        const auto visualisation = uR"(NodeName Colour with palette = "%1")"_s.arg(u::escapeQuotes(palette));

        return changeVisuals([&] { _document->update({}, {visualisation}); });
    }

    bool settled() const
    {
        return !_graphDisplay->property("transitioning").toBool() && !_document->busy();
    }

    // The window once it has stopped changing; each grab synchronises the renderer
    // with the display, which is what starts any transition the display has pending
    QImage grab()
    {
        QImage previous;
        QImage image;
        const bool still = QTest::qWaitFor([&]
        {
            previous = image;
            image = _window->grabWindow();
            return settled() && image == previous;
        }, Timeout);

        return still ? image : QImage{};
    }

    // Each node colour is a primary or secondary colour; shading varies its brightness
    // and the specular highlights wash it out, but the channels that are on stay on
    static bool showsColour(const QColor& pixel, const QColor& color)
    {
        const auto brightest = std::max({pixel.redF(), pixel.greenF(), pixel.blueF()});
        if(brightest < 0.2f)
            return false;

        const auto threshold = brightest * 0.6f;
        const bool red = pixel.redF() > threshold;
        const bool green = pixel.greenF() > threshold;
        const bool blue = pixel.blueF() > threshold;

        if(red && green && blue)
            return false;

        return red == (color.redF() > 0.5f) && green == (color.greenF() > 0.5f) && blue == (color.blueF() > 0.5f);
    }

    static Patch find(const QImage& image, const QColor& color)
    {
        Patch patch;
        QPointF sum;

        for(int y = 0; y < image.height(); y++)
        {
            for(int x = 0; x < image.width(); x++)
            {
                if(showsColour(image.pixelColor(x, y), color))
                {
                    patch._pixels++;
                    sum += QPointF(x, y);
                }
            }
        }

        if(patch._pixels > 0)
            patch._centre = (sum / patch._pixels) / image.devicePixelRatio();

        return patch;
    }

    static QColor pixelAt(const QImage& image, QPointF position)
    {
        return image.pixelColor((position * image.devicePixelRatio()).toPoint());
    }

    // Call from initTestCase, as QtTest replaces any message handler installed before then
    static void recordOpenGLMessages()
    {
        previousMessageHandler() = qInstallMessageHandler(recordOpenGLMessage);
    }

    static QStringList takeOpenGLMessages()
    {
        const QMutexLocker locker(&openGLMessagesMutex());
        return std::exchange(openGLMessagesStore(), {});
    }

private:
    static QMutex& openGLMessagesMutex()
    {
        static QMutex mutex;
        return mutex;
    }

    static QStringList& openGLMessagesStore()
    {
        static QStringList messages;
        return messages;
    }

    static QtMessageHandler& previousMessageHandler()
    {
        static QtMessageHandler handler = nullptr;
        return handler;
    }

    static void recordOpenGLMessage(QtMsgType type, const QMessageLogContext& context, const QString& message)
    {
        if(message.startsWith(u"OpenGL:"_s))
        {
            const QMutexLocker locker(&openGLMessagesMutex());
            openGLMessagesStore().append(message);
        }

        if(previousMessageHandler() != nullptr)
            previousMessageHandler()(type, context, message);
    }
};

#endif // RENDERTEST_H
