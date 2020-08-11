#include "variantlessthan.h"

#include <QVariant>
#include <QDate>
#include <QtGlobal>

bool qqsfpm::isVariantLessThan(const QVariant& left, const QVariant& right)
{
    if(left.userType() == QVariant::Invalid)
        return false;

    if(right.userType() == QVariant::Invalid)
        return true;

    switch(left.userType())
    {
    case QVariant::Int:
        return left.toInt() < right.toInt();
    case QVariant::UInt:
        return left.toUInt() < right.toUInt();
    case QVariant::LongLong:
        return left.toLongLong() < right.toLongLong();
    case QVariant::ULongLong:
        return left.toULongLong() < right.toULongLong();
    case QMetaType::Float:
        return left.toFloat() < right.toFloat();
    case QVariant::Double:
        return left.toDouble() < right.toDouble();
    case QVariant::Char:
        return left.toChar() < right.toChar();
    case QVariant::Date:
        return left.toDate() < right.toDate();
    case QVariant::Time:
        return left.toTime() < right.toTime();
    case QVariant::DateTime:
        return left.toDateTime() < right.toDateTime();
    case QVariant::String:
    default:
        return left.toString().compare(right.toString(), Qt::CaseSensitive) < 0;
    }
}
