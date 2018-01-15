#ifndef OPENGLFUNCTIONS_H
#define OPENGLFUNCTIONS_H

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLExtensions>
#include <QString>

#include <memory>

class OpenGLFunctions : public QOpenGLFunctions_3_3_Core
{
public:
    void resolveOpenGLFunctions();

    bool hasSampleShading() const { return _sampleShadingExtension != nullptr; }
    inline void glMinSampleShading(GLfloat value)
    {
        if(hasSampleShading())
            _sampleShadingExtension->glMinSampleShadingARB(value);
    }

    static bool hasOpenGLSupport();
    static QString vendor();
    static QString info();

    static void setDefaultFormat();
    static QSurfaceFormat defaultFormat();

private:
    std::unique_ptr<QOpenGLExtension_ARB_sample_shading> _sampleShadingExtension;
};

// MacOS's glext.h is rubbish
#ifndef GL_ARB_sample_shading
#define GL_ARB_sample_shading 1
#define GL_SAMPLE_SHADING_ARB             0x8C36
#define GL_MIN_SAMPLE_SHADING_VALUE_ARB   0x8C37
typedef void (APIENTRYP PFNGLMINSAMPLESHADINGARBPROC) (GLfloat value);
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void APIENTRY glMinSampleShadingARB (GLfloat value);
#endif
#endif /* GL_ARB_sample_shading */

#endif // OPENGLFUNCTIONS_H
