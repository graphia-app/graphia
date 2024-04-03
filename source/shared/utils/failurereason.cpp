/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "failurereason.h"
#include "source_location.h"

#include "build_defines.h"

using namespace Qt::Literals::StringLiterals;

void FailureReason::setFailureReason(const QString& failureReason)
{
    _failureReason = failureReason;
}

const QString& FailureReason::failureReason() const
{
    return _failureReason;
}

void FailureReason::setGenericFailureReason(const source_location& location)
{
    _failureReason = u"Failure at %1:%2\n%3\nBuild %4"_s
        .arg(location.file_name())
        .arg(location.line())
        .arg(location.function_name(), QStringLiteral(VERSION));
}
