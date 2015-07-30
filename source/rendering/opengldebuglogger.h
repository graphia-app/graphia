#ifndef OPENGLDEBUGLOGGER_H
#define OPENGLDEBUGLOGGER_H

#include <QObject>

class QOpenGLDebugLogger;
class QOpenGLDebugMessage;

class OpenGLDebugLogger : public QObject
{
    Q_OBJECT
public:
    explicit OpenGLDebugLogger(QObject* parent = nullptr);
    virtual ~OpenGLDebugLogger();

private:
    int _debugLevel = 0;
    QOpenGLDebugLogger* _logger = nullptr;

private slots:
    void onMessageLogged(const QOpenGLDebugMessage& message);
};

#endif // OPENGLDEBUGLOGGER_H
