#ifndef OPENGLFUNCTIONS_H
#define OPENGLFUNCTIONS_H

#include <QOpenGLFunctions_3_3_Core>
#include <QDebug>

class OpenGLFunctions : public QOpenGLFunctions_3_3_Core
{
public:
    OpenGLFunctions() :
        QOpenGLFunctions_3_3_Core()
    {}

    void resolveOpenGLFunctions();

    static bool checkOpenGLSupport();
};

#endif // OPENGLFUNCTIONS_H
