#ifndef ENRICHMENTCALCULATOR_H
#define ENRICHMENTCALCULATOR_H

#include <QString>

#include "shared/graph/elementid_containers.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/igraph.h"
#include "shared/commands/icommandmanager.h"
#include "enrichmenttablemodel.h"

class EnrichmentCalculator
{
public:
    EnrichmentCalculator();
    static double Fishers(int a, int b, int c, int d);
    static std::vector<double> doRandomSampling(int totalGenes, double expectedFrequency);
    static EnrichmentTableModel::Table overRepAgainstEachAttribute(QString attributeAName, QString attributeBName, IGraphModel *graphModel, ICommand &command);
};

#endif // ENRICHMENTCALCULATOR_H
