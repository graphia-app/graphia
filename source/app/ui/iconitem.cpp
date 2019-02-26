#include "iconitem.h"

IconItem::IconItem(QQuickItem* parent) : QQuickPaintedItem(parent)
{
    // Default size
    setWidth(24.0);
    setHeight(24.0);

    connect(this, &IconItem::enabledChanged, [this] { update(); });
    connect(this, &IconItem::iconNameChanged, [this] { update(); });
    connect(this, &IconItem::onChanged, [this] { update(); });
}

void IconItem::paint(QPainter* painter)
{
    _icon.paint(painter, boundingRect().toRect(),
        Qt::AlignCenter, isEnabled() ? QIcon::Normal : QIcon::Disabled,
        _on ? QIcon::On : QIcon::Off);
}

void IconItem::setIconName(const QString& iconName)
{
    bool wasValid = valid();

    _iconName = iconName;
    _icon = QIcon::fromTheme(iconName);

    if(wasValid != valid())
        emit validChanged();

    emit iconNameChanged();
}
