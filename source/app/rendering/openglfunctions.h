#ifndef OPENGLFUNCTIONS_H
#define OPENGLFUNCTIONS_H

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLExtensions>
#include <QString>

class OpenGLFunctions : public QOpenGLFunctions_3_3_Core
{
public:
    ~OpenGLFunctions() override;

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
    QOpenGLExtension_ARB_sample_shading* _sampleShadingExtension = nullptr;
};

#endif // OPENGLFUNCTIONS_H
