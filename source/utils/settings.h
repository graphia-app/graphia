 #ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings
{
private:
    QSettings* qSettings;

public:
    Settings();
    ~Settings();

    template<typename T>
    void setValue(QString key, T t){
        qSettings->setValue(key,t);
    }
    template<typename T>
    T value(QString key){
        return qSettings->value(key);
    }
};

#endif // SETTINGS_H
