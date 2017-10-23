#include "screenshotrenderer.h"

ScreenshotRenderer::ScreenshotRenderer() :
    QObject(),
    GraphRendererCore()
{
}

GLuint ScreenshotRenderer::sdfTexture() const
{
    return 0;
}
