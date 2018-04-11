#include "enrichmentcalculator.h"

#include <cmath>
#include <set>
#include <vector>
#include <map>

#include <QDebug>

#include "shared/utils/container.h"
#include "shared/attributes/iattribute.h"
#include "shared/utils/random.h"

static double combineLogs(double n, double r)
{
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
double EnrichmentCalculator::Fishers(int a, int b, int c, int d)
{
    double ab = a + b;
    double cd = c + d;
    double ac = a + c;
    double bd = b + d;

    double leftPval  = 0.0;
    double rightPval = 0.0;
    double twoPval   = 0.0;

    // range of variation
    double lm = (ac < cd) ? 0.0 : ac - cd;
    double um = (ac < ab) ? ac  : ab;

    // Fisher's exact test
    double crit = hyperGeometricProb(a, ab, cd, ac, bd);

    leftPval = rightPval = twoPval = 0.0;
    for(double x = lm; x <= um; x++)
    {
        double prob = hyperGeometricProb(x, ab, cd, ac, bd);
        if(x <= a)
            leftPval += prob;
        if(x >= a)
            rightPval += prob;
        if(prob <= crit)
            twoPval += prob;
    }

    return twoPval;
}

EnrichmentTableModel::Table EnrichmentCalculator::overRepAgainstEachAttribute(QString attributeAName, QString attributeBName,
                                                       IGraphModel* graphModel, ICommand& command)
{
    // Count of attribute values within the attribute
    std::map<QString, int> attributeValueEntryCountATotal;
    std::map<QString, int> attributeValueEntryCountBTotal;
    EnrichmentTableModel::Table tableModel;

    for(auto nodeId : graphModel->graph().nodeIds())
    {
        const auto& stringAttributeValue = graphModel->attributeByName(attributeAName)->stringValueOf(nodeId);
        const auto& stringAttributeForValue = graphModel->attributeByName(attributeBName)->stringValueOf(nodeId);
        ++attributeValueEntryCountATotal[stringAttributeValue];
        ++attributeValueEntryCountBTotal[stringAttributeForValue];
    }

    int n = 0;
    int selectedInCategory = 0;
    int r1 = 0;
    double fobs = 0.0;
    double fexp = 0.0;
    double overRepresentation = 0.0;
    std::vector<double> stdevs(4);
    double expectedNo = 0.0;
    double expectedDev = 0.0;
    double expectedOverrep = 0.0;
    double zScore = 0.0;

    // Comparing

    int progress = 0;
    int iterCount = static_cast<int>(attributeValueEntryCountBTotal.size()
                                     * attributeValueEntryCountATotal.size());

    // Get all the nodeIds for each AttributeFor value
    // Maps of vectors uhoh.
    auto* attributeB = graphModel->attributeByName(attributeBName);
    std::map<QString, std::vector<NodeId>> nodeIdsForAttributeValue;
    for(auto nodeId : graphModel->graph().nodeIds())
        nodeIdsForAttributeValue[attributeB->stringValueOf(nodeId)].push_back(nodeId);

    for(auto& attributeValueFor : u::keysFor(attributeValueEntryCountBTotal))
    {
        auto& selectedNodes = nodeIdsForAttributeValue[attributeValueFor];

        for(auto& attributeValue : u::keysFor(attributeValueEntryCountATotal))
        {
            EnrichmentTableModel::Row row(9);
            command.setProgress(progress / iterCount);
            progress++;

            n = graphModel->graph().numNodes();

            auto* attribute = graphModel->attributeByName(attributeAName);
            selectedInCategory = 0;
            for(auto nodeId : selectedNodes)
            {
                if(attribute->stringValueOf(nodeId) == attributeValue)
                    selectedInCategory++;
            }

            r1 = attributeValueEntryCountATotal[attributeValue];
            fobs = static_cast<double>(selectedInCategory) / static_cast<double>(selectedNodes.size());
            fexp = static_cast<double>(r1) / static_cast<double>(n);
            overRepresentation = fobs / fexp;
            stdevs = doRandomSampling(static_cast<int>(selectedNodes.size()), fexp);

            expectedNo = (static_cast<double>(r1) / static_cast<double>(n)
                                                * static_cast<double>(selectedNodes.size()));
            expectedDev = stdevs[0] * static_cast<double>(selectedNodes.size());
            expectedOverrep = stdevs[3];
            zScore = (overRepresentation - expectedOverrep) / stdevs[1];

            auto nonSelectedInCategory = r1 - selectedInCategory;
            auto c1 = static_cast<int>(selectedNodes.size());
            auto selectedNotInCategory = c1 - selectedInCategory;
            auto c2 = n - c1;
            auto nonSelectedNotInCategory = c2 - nonSelectedInCategory;
            auto f = Fishers(selectedInCategory, nonSelectedInCategory, selectedNotInCategory, nonSelectedNotInCategory);

            row[0] = attributeValueFor;
            row[1] = attributeValue;
            row[2] = QString::number(selectedInCategory) + " / " + QString::number(selectedNodes.size());
            row[3] = QString::number(expectedNo, 'f', 2) + " / " + QString::number(selectedNodes.size());
            row[4] = QString::number(expectedNo, 'f', 2) + " / " + QString::number(selectedNodes.size()) + " ± " + QString::number(expectedDev);
            row[5] = QString::number(selectedInCategory / expectedNo, 'f', 2);
            row[6] = QString::number(f, 'f', 2);

            tableModel.push_back(row);
        }
    }

    return tableModel;
}

std::vector<double> EnrichmentCalculator::doRandomSampling(int totalGenes, double expectedFrequency)
{
    const int NUMBER_OF_TRIALS = 1000;
    double observed[NUMBER_OF_TRIALS];
    double overRepresentation[NUMBER_OF_TRIALS];
    double observationAvg = 0;
    double overRepresentationAvg = 0;
    double observationStdDev = 0;
    double overRepresentationStdDev = 0;

    for(int i = 0; i < NUMBER_OF_TRIALS; i++)
    {
        int hits = 0;
        for(int j = 0; j < totalGenes; j++)
        {
            if(static_cast<double>(u::rand(0.0f, 1.0f)) <= expectedFrequency)
                hits++;
        }

        observed[i] = hits / static_cast<double>(totalGenes);
        overRepresentation[i] = observed[i] / expectedFrequency;
        observationAvg += observed[i];
        overRepresentationAvg += overRepresentation[i];
    }

    observationAvg = observationAvg / static_cast<double>(NUMBER_OF_TRIALS);
    overRepresentationAvg = overRepresentationAvg / static_cast<double>(NUMBER_OF_TRIALS);

    for(int i = 0; i < NUMBER_OF_TRIALS; i++)
    {
        observationStdDev += (observed[i] - observationAvg) * (observed[i] - observationAvg);
        overRepresentationStdDev += (overRepresentation[i] - overRepresentationAvg) * (overRepresentation[i] - overRepresentationAvg);
    }

    observationStdDev = sqrt(observationStdDev / static_cast<double>(NUMBER_OF_TRIALS));
    overRepresentationStdDev = sqrt(overRepresentationStdDev / static_cast<double>(NUMBER_OF_TRIALS));

    return { observationStdDev, overRepresentationStdDev, observationAvg, overRepresentationAvg };
}

