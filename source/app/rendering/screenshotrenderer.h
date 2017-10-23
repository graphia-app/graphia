#ifndef SCREENSHOTRENDERER_H
#define SCREENSHOTRENDERER_H

#include "graphrenderercore.h"

#include <QObject>

class ScreenshotRenderer :
        public QObject,
        public GraphRendererCore
{
    Q_OBJECT

public:
    ScreenshotRenderer();
    virtual ~ScreenshotRenderer() = default;

private:
    GLuint sdfTexture() const override;
};

#endif // SCREENSHOTRENDERER_H
