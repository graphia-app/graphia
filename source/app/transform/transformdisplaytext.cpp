/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "transformdisplaytext.h"

#include <QObject>

// %1 is the transform
QString TransformDisplayText::pinned()          { return QObject::tr("📌 %1"); }

// %1 is the transform so far, %2 its attributes, parameters or condition
QString TransformDisplayText::usingAttributes() { return QObject::tr("%1 using %2"); }
QString TransformDisplayText::withParameters()  { return QObject::tr("%1 with %2"); }
QString TransformDisplayText::whereCondition()  { return QObject::tr("%1 where %2"); }

// %1 is the parameter's name, %2 its value
QString TransformDisplayText::parameter()       { return QObject::tr("%1 = %2", "a transform parameter's value"); }

TransformDisplayText::Op TransformDisplayText::op(ConditionFnOp::Equality op)
{
    switch(op)
    {
    case ConditionFnOp::Equality::Equal:    return {._name = QObject::tr("="), ._phrase = QObject::tr("%1 = %2", "a condition")};
    case ConditionFnOp::Equality::NotEqual: return {._name = QObject::tr("≠"), ._phrase = QObject::tr("%1 ≠ %2")};
    }

    return {};
}

TransformDisplayText::Op TransformDisplayText::op(ConditionFnOp::Numerical op)
{
    switch(op)
    {
    case ConditionFnOp::Numerical::LessThan:            return {._name = QObject::tr("<"), ._phrase = QObject::tr("%1 < %2")};
    case ConditionFnOp::Numerical::GreaterThan:         return {._name = QObject::tr(">"), ._phrase = QObject::tr("%1 > %2")};
    case ConditionFnOp::Numerical::LessThanOrEqual:     return {._name = QObject::tr("≤"), ._phrase = QObject::tr("%1 ≤ %2")};
    case ConditionFnOp::Numerical::GreaterThanOrEqual:  return {._name = QObject::tr("≥"), ._phrase = QObject::tr("%1 ≥ %2")};
    }

    return {};
}

TransformDisplayText::Op TransformDisplayText::op(ConditionFnOp::String op)
{
    switch(op)
    {
    case ConditionFnOp::String::Includes:
        return {._name = QObject::tr("Includes"), ._phrase = QObject::tr("%1 Includes %2")};
    case ConditionFnOp::String::Excludes:
        return {._name = QObject::tr("Excludes"), ._phrase = QObject::tr("%1 Excludes %2")};
    case ConditionFnOp::String::Starts:
        return {._name = QObject::tr("Starts With"), ._phrase = QObject::tr("%1 Starts With %2")};
    case ConditionFnOp::String::Ends:
        return {._name = QObject::tr("Ends With"), ._phrase = QObject::tr("%1 Ends With %2")};
    case ConditionFnOp::String::MatchesRegex:
        return {._name = QObject::tr("Matches Regex"), ._phrase = QObject::tr("%1 Matches Regex %2")};
    case ConditionFnOp::String::MatchesRegexCaseInsensitive:
        return {._name = QObject::tr("Matches Case Insensitive Regex"), ._phrase = QObject::tr("%1 Matches Case Insensitive Regex %2")};
    }

    return {};
}

TransformDisplayText::Op TransformDisplayText::op(ConditionFnOp::Logical op)
{
    switch(op)
    {
    case ConditionFnOp::Logical::Or:    return {._name = QObject::tr("or"), ._phrase = QObject::tr("%1 or %2")};
    case ConditionFnOp::Logical::And:   return {._name = QObject::tr("and"), ._phrase = QObject::tr("%1 and %2")};
    }

    return {};
}

TransformDisplayText::Op TransformDisplayText::op(ConditionFnOp::Unary op)
{
    switch(op)
    {
    case ConditionFnOp::Unary::HasValue: return {._name = QObject::tr("Has Value"), ._phrase = QObject::tr("%1 Has Value")};
    }

    return {};
}
