#include "enrichmentcalculator.h"

#include <cmath>
#include <set>
#include <vector>
#include <map>

#include <QDebug>

#include "shared/utils/container.h"
#include "shared/attributes/iattribute.h"
#include "shared/utils/random.h"

static double logComb(double n, double r)
{
    return std::lgamma(n + 1) - std::lgamma(r + 1) - std::lgamma(n - r + 1);
}

static double hyperGeometricProb(double x, double r1, double r2, double c1, double c2)
{
    return std::exp( logComb(r1, x) + logComb(r2, c1 - x) - logComb( c1 + c2, c1) );
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
    for (double x = lm; x <= um; x++)
    {
        double prob = hyperGeometricProb(x, ab, cd, ac, bd);
        if (x <= a) leftPval += prob;
        if (x >= a) rightPval += prob;
        if (prob <= crit) twoPval += prob;
    }

    return twoPval;
}

void EnrichmentCalculator::overRepAgainstEachAttribute(NodeIdSet& selectedNodeIds, QString attributeAgainst, IGraphModel* graphModel)
{
    // Count of attribute values within the attribute
    std::map<QString, int> attributeValueEntryCountTotal;
    std::map<QString, int> attributeValueEntryCountSelected;

    for(auto nodeId : graphModel->graph().nodeIds())
    {
        auto& stringAttributeValue = graphModel->attributeByName(attributeAgainst)->stringValueOf(nodeId);
        ++attributeValueEntryCountTotal[stringAttributeValue];
    }

    for(auto nodeId : selectedNodeIds)
    {
        auto& stringAttributeValue = graphModel->attributeByName(attributeAgainst)->stringValueOf(nodeId);
        ++attributeValueEntryCountSelected[stringAttributeValue];
    }


    int n = 0;
    int selectedInCategory = 0;
    int r1 = 0;
    double fobs = 0.0;
    double fexp = 0.0;
    double overRep = 0.0;
    std::vector<double> stdevs(4);
    double expectedNo = 0.0;
    double expectedDev = 0.0;
    double expectedOverrep = 0.0;
    double zScore = 0.0;

    // Cluster is against attribute!
    for ( auto& attributeValue : u::keysFor(attributeValueEntryCountSelected) )
    {
        n = graphModel->graph().numNodes();

        selectedInCategory = attributeValueEntryCountSelected[attributeValue];
        
        r1 = attributeValueEntryCountTotal[attributeValue];
        fobs = (double) selectedInCategory / (double) selectedNodeIds.size();
        fexp = (double) r1 / (double) n;
        overRep = fobs / fexp;
        stdevs = doRandomSampling(selectedNodeIds.size(), fexp);

        expectedNo = (((double) r1 / (double) n) * (double) selectedNodeIds.size());
        expectedDev = ((stdevs[0]) * (double) selectedNodeIds.size());
        expectedOverrep = stdevs[3];
        zScore = (overRep - expectedOverrep) / stdevs[1];

        qDebug() << attributeValue;
        qDebug() << "Observed" << selectedInCategory << "/" << selectedNodeIds.size();
        qDebug() << "Expected" << expectedNo << "/" << selectedNodeIds.size();
        qDebug() << "ExpectedTrial" << expectedNo << "/" << selectedNodeIds.size() << "±" << expectedDev;
        qDebug() << "FObs" << fobs;
        qDebug() << "FExp" << fexp;
        qDebug() << "OverRep" << selectedInCategory / expectedNo;
        qDebug() << "ZScore" << zScore;
    }
}

std::vector<double> EnrichmentCalculator::doRandomSampling(int totalGenes, double expectedFrequency)
{
    const int NUMBER_OF_TRIALS = 1000;
    double trialObs[NUMBER_OF_TRIALS];
    double trialOverRep[NUMBER_OF_TRIALS];
    double trialObsAvg = 0;
    double trialOverRepAvg = 0;
    double trialObsStdev = 0;
    double trialOverRepStdev = 0;

    for (int i = 0; i < NUMBER_OF_TRIALS; i++)
    {
        int hits = 0;
        for (int j = 0; j < totalGenes; j++)
            if (u::rand(0.0f, 1.0f) <= expectedFrequency)
                hits++;

        trialObs[i] = hits / (double)totalGenes;
        trialOverRep[i] = trialObs[i] / expectedFrequency;
        trialObsAvg += trialObs[i];
        trialOverRepAvg += trialOverRep[i];
    }

    trialObsAvg = trialObsAvg / (double)NUMBER_OF_TRIALS;
    trialOverRepAvg = trialOverRepAvg / (double)NUMBER_OF_TRIALS;

    for (int i = 0; i < NUMBER_OF_TRIALS; i++)
    {
        trialObsStdev += (trialObs[i] - trialObsAvg) * (trialObs[i] - trialObsAvg);
        trialOverRepStdev += (trialOverRep[i] - trialOverRepAvg) * (trialOverRep[i] - trialOverRepAvg);
    }

    trialObsStdev = sqrt(trialObsStdev / (double)NUMBER_OF_TRIALS);
    trialOverRepStdev = sqrt(trialOverRepStdev / (double)NUMBER_OF_TRIALS);

    return { trialObsStdev, trialOverRepStdev, trialObsAvg, trialOverRepAvg };
}

