#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QSettings>
#include "../utils/singleton.h"

class Preferences : public Singleton<Preferences>
{
public:
    Preferences();
    QSettings* settings;
};

#endif // PREFERENCES_H
