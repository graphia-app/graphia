#include "elementtype.h"

#include <QObject>

QString elementTypeAsString(ElementType elementType)
{
    switch(elementType)
    {
    case ElementType::Node:      return QObject::tr("Node");
    case ElementType::Edge:      return QObject::tr("Edge");
    case ElementType::Component: return QObject::tr("Component");
    default: break;
    }

    return {};
}
