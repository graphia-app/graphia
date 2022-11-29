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

#include "enrichmentcalculator.h"

#include <cmath>
#include <set>
#include <vector>
#include <map>
#include <random>
#include <array>

#include "shared/graph/igraphmodel.h"
#include "shared/graph/igraph.h"
#include "shared/commands/icommandmanager.h"

#include "shared/utils/container.h"
#include "shared/utils/msvcwarningsuppress.h"

#include "shared/attributes/iattribute.h"

static double combineLogs(double n, double r)
{
    // NOLINTNEXTLINE concurrency-mt-unsafe
    return std::lgamma(n + 1) - std::lgamma(r + 1) - std::lgamma(n - r + 1);
}

static double hyperGeometricProb(double x, double r1, double r2, double c1, double c2)
{
    return std::exp(combineLogs(r1, x) + combineLogs(r2, c1 - x) - combineLogs( c1 + c2, c1));
}

/*
 *  A: Selected In Category
 *  B: Not Selected In Category
 *  C: Selected NOT In Category
 *  D: Not Selected NOT In Category
 */
double EnrichmentCalculator::fishers(size_t a, size_t b, size_t c, size_t d)
{
    auto ab = static_cast<double>(a + b);
    auto cd = static_cast<double>(c + d);
    auto ac = static_cast<double>(a + c);
    auto bd = static_cast<double>(b + d);

    double twoPval = 0.0;

    // range of variation
    const double lm = (ac < cd) ? 0.0 : ac - cd;
    const double um = (ac < ab) ? ac  : ab;

    // Fisher's exact test
    const double crit = hyperGeometricProb(static_cast<double>(a), ab, cd, ac, bd);

    for(auto x = static_cast<int>(lm); x <= static_cast<int>(um); x++)
    {
        const double prob = hyperGeometricProb(static_cast<double>(x), ab, cd, ac, bd);

        if(prob <= crit)
            twoPval += prob;
    }

    return twoPval;
}

EnrichmentTableModel::Table EnrichmentCalculator::overRepAgainstEachAttribute(
    const QString& attributeAName, const QString& attributeBName,
    IGraphModel* graphModel, ICommand& command)
{
    // Count of attribute values within the attribute
    std::map<QString, size_t> attributeValueEntryCountATotal;
    std::map<QString, size_t> attributeValueEntryCountBTotal;
    EnrichmentTableModel::Table tableModel;

    for(auto nodeId : graphModel->graph().nodeIds())
    {
        const auto* attributeA = graphModel->attributeByName(attributeAName);
        const auto* attributeB = graphModel->attributeByName(attributeBName);
        const auto& stringAttributeA = attributeA->stringValueOf(nodeId);
        const auto& stringAttributeB = attributeB->stringValueOf(nodeId);

        if(!stringAttributeA.isEmpty())
            ++attributeValueEntryCountATotal[stringAttributeA];

        if(!stringAttributeB.isEmpty())
            ++attributeValueEntryCountBTotal[stringAttributeB];
    }

    // Comparing

    uint64_t progress = 0;
    auto iterations = static_cast<uint64_t>(attributeValueEntryCountBTotal.size() *
        attributeValueEntryCountATotal.size());

    // Get all the nodeIds for each AttributeFor value
    // Maps of vectors uhoh.
    const auto* attributeA = graphModel->attributeByName(attributeAName);
    const auto* attributeB = graphModel->attributeByName(attributeBName);

    std::map<QString, std::vector<NodeId>> nodeIdsForAttributeValue;
    for(auto nodeId : graphModel->graph().nodeIds())
    {
        const auto& value = attributeA->stringValueOf(nodeId);

        if(!value.isEmpty())
            nodeIdsForAttributeValue[value].push_back(nodeId); // clazy:exclude=reserve-candidates
    }

    for(auto& attributeValueA : u::keysFor(attributeValueEntryCountATotal))
    {
        auto& selectedNodes = nodeIdsForAttributeValue[attributeValueA];

        for(auto& attributeValueB : u::keysFor(attributeValueEntryCountBTotal))
        {
            EnrichmentTableModel::Row row(EnrichmentTableModel::Results::NumResultColumns);
            command.setProgress(static_cast<int>(progress * 100U / iterations));
            progress++;

            auto n = graphModel->graph().numNodes();

            size_t selectedInCategory = 0;
            for(auto nodeId : selectedNodes)
            {
                if(attributeB->stringValueOf(nodeId) == attributeValueB)
                    selectedInCategory++;
            }

            auto r1 = attributeValueEntryCountBTotal[attributeValueB];
            auto fexp = static_cast<double>(r1) / static_cast<double>(n);
            auto stdevs = doRandomSampling(static_cast<int>(selectedNodes.size()), fexp);

            auto expectedNo = (static_cast<double>(r1) / static_cast<double>(n)) *
                static_cast<double>(selectedNodes.size());
            auto expectedDev = stdevs[0] * static_cast<double>(selectedNodes.size());

            auto nonSelectedInCategory = r1 - selectedInCategory;
            auto c1 = selectedNodes.size();
            auto selectedNotInCategory = c1 - selectedInCategory;
            auto c2 = n - c1;
            auto nonSelectedNotInCategory = c2 - nonSelectedInCategory;
            auto f = fishers(selectedInCategory, nonSelectedInCategory, selectedNotInCategory, nonSelectedNotInCategory);

            row[EnrichmentTableModel::Results::SelectionA] = attributeValueA;
            row[EnrichmentTableModel::Results::SelectionB] = attributeValueB;
            row[EnrichmentTableModel::Results::Observed] = QStringLiteral("%1 of %2")
                .arg(selectedInCategory)
                .arg(selectedNodes.size());
            row[EnrichmentTableModel::Results::ExpectedTrial] = QStringLiteral("%1 ± %2 of %3")
                .arg(QString::number(expectedNo, 'f', 2),
                QString::number(expectedDev, 'f', 2),
                QString::number(selectedNodes.size()));
            row[EnrichmentTableModel::Results::OverRep] = static_cast<double>(selectedInCategory) / expectedNo;
            row[EnrichmentTableModel::Results::Fishers] = f;
            row[EnrichmentTableModel::Results::BonferroniAdjusted] =
                std::min(1.0, f * static_cast<double>(attributeValueEntryCountBTotal.size()));

            tableModel.push_back(row);
        }
    }

    return tableModel;
}

MSVC_WARNING_SUPPRESS_NEXTLINE(6262)
std::vector<double> EnrichmentCalculator::doRandomSampling(int sampleCount, double expectedFrequency)
{
    const size_t NUMBER_OF_TRIALS = 1000;
    std::array<double, NUMBER_OF_TRIALS> observed{};
    std::array<double, NUMBER_OF_TRIALS> overRepresentation{};
    double observationAvg = 0.0;
    double overRepresentationAvg = 0.0;
    double observationStdDev = 0.0;
    double overRepresentationStdDev = 0.0;

    std::random_device rd;
    auto seededGenerator = std::mt19937(rd());
    auto distribution = std::uniform_real_distribution<>(0.0f, 1.0f);

    for(size_t i = 0; i < NUMBER_OF_TRIALS; i++)
    {
        int hits = 0;
        for(int j = 0; j < sampleCount; j++)
        {
            if(static_cast<double>(distribution(seededGenerator)) <= expectedFrequency)
                hits++;
        }

        observed.at(i) = hits / static_cast<double>(sampleCount);
        overRepresentation.at(i) = observed.at(i) / expectedFrequency;
        observationAvg += observed.at(i);
        overRepresentationAvg += overRepresentation.at(i);
    }

    observationAvg = observationAvg / static_cast<double>(NUMBER_OF_TRIALS);
    overRepresentationAvg = overRepresentationAvg / static_cast<double>(NUMBER_OF_TRIALS);

    for(size_t i = 0; i < NUMBER_OF_TRIALS; i++)
    {
        observationStdDev += (observed.at(i) - observationAvg) * (observed.at(i) - observationAvg);
        overRepresentationStdDev += (overRepresentation.at(i) - overRepresentationAvg) * (overRepresentation.at(i) - overRepresentationAvg);
    }

    observationStdDev = std::sqrt(observationStdDev / static_cast<double>(NUMBER_OF_TRIALS));
    overRepresentationStdDev = std::sqrt(overRepresentationStdDev / static_cast<double>(NUMBER_OF_TRIALS));

    return { observationStdDev, overRepresentationStdDev, observationAvg, overRepresentationAvg };
}

