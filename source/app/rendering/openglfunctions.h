#ifndef OPENGLFUNCTIONS_H
#define OPENGLFUNCTIONS_H

#include <QOpenGLFunctions_3_3_Core>
#include <QString>

class OpenGLFunctions : public QOpenGLFunctions_3_3_Core
{
public:
    OpenGLFunctions() :
        QOpenGLFunctions_3_3_Core()
    {}

    void resolveOpenGLFunctions();

    static bool hasOpenGLSupport();
    static QString info();
};

#endif // OPENGLFUNCTIONS_H
