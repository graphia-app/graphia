#ifndef LAYOUTSETTINGS_H
#define LAYOUTSETTINGS_H

#include <map>
#include <QObject>
#include <QString>
#include <QQmlListProperty>
#include <memory>

class LayoutParam : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER name NOTIFY nameChanged)
    Q_PROPERTY(float val MEMBER value NOTIFY valueChanged)
    Q_PROPERTY(float min MEMBER min NOTIFY minChanged)
    Q_PROPERTY(float max MEMBER max NOTIFY maxChanged)
public:
    LayoutParam(QString inname, float inmin, float inmax, float invalue);
    LayoutParam();
    QString name;
    float value;
    float min;
    float max;
signals:
    void nameChanged();
    void valueChanged();
    void minChanged();
    void maxChanged();
};

class LayoutSettings : public QObject
{
    Q_OBJECT

public:

    LayoutSettings();

    bool setParamValue(QString key, float value);

    std::shared_ptr<LayoutParam> getParam(QString key);

    std::map<QString, std::shared_ptr<LayoutParam>>& paramMap();

    bool registerParam(QString name, float min, float max, float value);

protected:


private:
    std::map<QString, std::shared_ptr<LayoutParam>> _params;
};

#endif // LAYOUTSETTINGS_H
