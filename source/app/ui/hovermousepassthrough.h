#ifndef HOVERMOUSEPASSTHROUGH_H
#define HOVERMOUSEPASSTHROUGH_H

#include <QObject>
#include <QQuickItem>
#include <QMouseEvent>

class HoverMousePassthrough : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool hovered MEMBER _hovered NOTIFY hoveredChanged)

private:
    bool _hovered = false;

public:
    explicit HoverMousePassthrough(QQuickItem* parent = nullptr) : QQuickItem(parent)
    {
        setAcceptHoverEvents(true);
    }

signals:
    void hoveredChanged();

protected:
    void hoverEnterEvent(QHoverEvent *event) override
    {
        event->ignore();
        _hovered = true;
        emit hoveredChanged();
    }

    void hoverLeaveEvent(QHoverEvent *event) override
    {
        event->ignore();
        _hovered = false;
        emit hoveredChanged();
    }
};

#endif // HOVERMOUSEPASSTHROUGH_H
