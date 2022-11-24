/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#ifndef VISUALISATIONCONFIGPARSER_H
#define VISUALISATIONCONFIGPARSER_H

#include "visualisationconfig.h"

#include <QString>

class VisualisationConfigParser
{
private:
    VisualisationConfig _result;
    bool _success = false;
    QString _failedInput;

public:
    bool parse(const QString& text, bool warnOnFailure = true);

    const VisualisationConfig& result() const { return _result; }
    bool success() const { return _success; }
    const QString& failedInput() const { return _failedInput; }

    static QString parseForDisplay(const QString& text);
};

#endif // VISUALISATIONCONFIGPARSER_H
