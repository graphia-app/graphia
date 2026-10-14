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

#ifndef TRANSFORMDISPLAYTEXT_H
#define TRANSFORMDISPLAYTEXT_H

#include "app/attributes/condtionfnops.h"

#include <QString>

// The text transforms are shown with; each is a whole phrase with its parts as arguments,
// so that a translation can put them in whatever order its language needs
namespace TransformDisplayText
{
QString pinned();
QString usingAttributes();
QString withParameters();
QString whereCondition();
QString parameter();

struct Op
{
    QString _name;      // For choosing the op, e.g. "Starts With"
    QString _phrase;    // %1 and %2 are its operands, e.g. "%1 Starts With %2"; unary ops have only %1
};

Op op(ConditionFnOp::Equality op);
Op op(ConditionFnOp::Numerical op);
Op op(ConditionFnOp::String op);
Op op(ConditionFnOp::Logical op);
Op op(ConditionFnOp::Unary op);
} // namespace TransformDisplayText

#endif // TRANSFORMDISPLAYTEXT_H
