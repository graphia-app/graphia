#ifndef REPORT_H
#define REPORT_H

#include <QObject>
#include <QString>

class Report : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString email MEMBER _email NOTIFY emailChanged)
    Q_PROPERTY(QString text MEMBER _text NOTIFY textChanged)

public:
    QString _email;
    QString _text;

signals:
    void emailChanged();
    void textChanged();
};

#endif // REPORT_H
