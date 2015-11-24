#include "layoutsettings.h"
#include "../utils/utils.h"

LayoutParam::LayoutParam(QString inname, float inmin, float inmax, float invalue){
    name = inname;
    min = inmin;
    max = inmax;
    value = invalue;
}

LayoutParam::LayoutParam(){
}

LayoutSettings::LayoutSettings()
{

}

bool LayoutSettings::setParamValue(QString key, float value)
{
    if ( u::contains(_params, key)){
        _params[key]->value = value;
        return true;
    } else {
        return false;
    }
}

std::shared_ptr<LayoutParam> LayoutSettings::getParam(QString key)
{
    return _params[key];
}

std::map<QString, std::shared_ptr<LayoutParam>>& LayoutSettings::paramMap()
{
    return _params;
}

bool LayoutSettings::registerParam(QString name, float min, float max, float value)
{
    _params.emplace(name, std::make_shared<LayoutParam>(name, min, max, value));
    return true;
}
