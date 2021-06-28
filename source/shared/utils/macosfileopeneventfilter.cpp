#include "macosfileopeneventfilter.h"

#include <QEvent>
#include <QFileOpenEvent>
#include <QString>

bool MacOsFileOpenEventFilter::eventFilter(QObject* object, QEvent* event)
{
    if(event->type() == QEvent::FileOpen)
    {
        QFileOpenEvent* fileOpenEvent = dynamic_cast<QFileOpenEvent*>(event);
        QString argument;

        if(!fileOpenEvent->url().isEmpty())
            argument = fileOpenEvent->url().toString();
        else if(!fileOpenEvent->file().isEmpty())
            argument = fileOpenEvent->file();

        emit externalOpen(argument); // NOLINT readability-misleading-indentation
    }

    return QObject::eventFilter(object, event);
}
