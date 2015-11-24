#include "preferences.h"


Preferences::Preferences()
{
    settings = new QSettings("Kajeka", "GraphTool");
}

