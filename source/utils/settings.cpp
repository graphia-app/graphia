#include "settings.h"

Settings::Settings()
{
    qSettings = new QSettings();
}

Settings::~Settings(void){

}
