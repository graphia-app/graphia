#ifndef ICONITEM_H
#define ICONITEM_H

#include <QQuickPaintedItem>
#include <QIcon>
#include <QString>

class IconItem : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged)
    Q_PROPERTY(bool on MEMBER _on NOTIFY onChanged)
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)

public:
    explicit IconItem(QQuickItem* parent = nullptr);

    void paint(QPainter *painter) override;

    QString iconName() const { return _iconName; }
    void setIconName(const QString& iconName);
    bool valid() const { return !_icon.isNull(); }

private:
    QString _iconName;
    bool _on = false;
    QIcon _icon;

signals:
    void iconNameChanged();
    void onChanged();
    void validChanged();
};

#endif // ICONITEM_H
