#ifndef HOVERMOUSEPASSTHROUGH_H
#define HOVERMOUSEPASSTHROUGH_H
#include <QObject>
#include <QQuickItem>
#include <QGuiApplication>
#include <QMouseEvent>

class HoverMousePassthrough : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool hovered MEMBER _hovered NOTIFY hoveredChanged)
private:
    bool _hovered;
public:
    explicit HoverMousePassthrough(QQuickItem *parent = 0) : QQuickItem(parent)
    {
        setAcceptHoverEvents(true);
    }
signals:
    void hoveredChanged();
protected:
    void hoverEnterEvent(QHoverEvent *event)
    {
        event->ignore();
        _hovered = true;
        hoveredChanged();
    }
    void hoverLeaveEvent(QHoverEvent *event)
    {
        event->ignore();
        _hovered = false;
        hoveredChanged();
    }
};
#endif // HOVERMOUSEPASSTHROUGH_H
