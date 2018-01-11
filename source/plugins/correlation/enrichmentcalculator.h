#ifndef ENRICHMENTCALCULATOR_H
#define ENRICHMENTCALCULATOR_H

#include <QString>

#include "shared/graph/elementid_containers.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/igraph.h"


class EnrichmentCalculator
{
public:
    EnrichmentCalculator();
    static double Fishers(int a, int b, int c, int d);
    static void overRepAgainstEachAttribute(NodeIdSet& selectedNodeIds, QString attributeAgainst, IGraphModel *graphModel);
    static std::vector<double> doRandomSampling(int totalGenes, double expectedFrequency);
};

#endif // ENRICHMENTCALCULATOR_H
