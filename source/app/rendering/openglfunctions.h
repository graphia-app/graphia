#ifndef OPENGLFUNCTIONS_H
#define OPENGLFUNCTIONS_H

#include <QOpenGLFunctions_4_0_Core>
#include <QString>

class OpenGLFunctions : public QOpenGLFunctions_4_0_Core
{
public:
    OpenGLFunctions() :
        QOpenGLFunctions_4_0_Core()
    {}

    void resolveOpenGLFunctions();

    static void setDefaultFormat();
    static bool hasOpenGLSupport();
    static QString vendor();
    static QString info();
};

#endif // OPENGLFUNCTIONS_H
