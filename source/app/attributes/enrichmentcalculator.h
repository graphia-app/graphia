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

#ifndef ENRICHMENTCALCULATOR_H
#define ENRICHMENTCALCULATOR_H
#include <vector>

#include "enrichmenttablemodel.h"

class QString;
class IGraphModel;
class ICommand;

class EnrichmentCalculator
{
public:
    static double fishers(size_t a, size_t b, size_t c, size_t d);
    static std::vector<double> doRandomSampling(int sampleCount, double expectedFrequency);
    static EnrichmentTableModel::Table overRepAgainstEachAttribute(const QString& attributeAName,
        const QString& attributeBName, IGraphModel* graphModel, ICommand& command);
};

#endif // ENRICHMENTCALCULATOR_H
