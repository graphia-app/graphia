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

#include "tests/common/testenvironment.h"

#include "plugins/correlation/correlation.h"
#include "plugins/correlation/correlationdatavector.h"
#include "plugins/correlation/correlationtype.h"
#include "plugins/correlation/featurescaling.h"
#include "plugins/correlation/knnprotograph.h"
#include "plugins/correlation/quantilenormaliser.h"
#include "plugins/correlation/softmaxnormaliser.h"

#include "shared/graph/edgelist.h"

#include <QTest>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include <cmath>
#include <map>
#include <memory>
#include <utility>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
using Rows = QList<QList<double>>;

// Each row is a vector, numbered by its index
template<typename DataVectors, typename T>
DataVectors dataVectors(const QList<QList<T>>& rows)
{
    DataVectors vectors;

    for(int i = 0; const auto& row : rows)
    {
        vectors.emplace_back(std::vector<T>{row.begin(), row.end()}, NodeId(i++));
        vectors.back().update();
    }

    return vectors;
}

const Rows continuousRows =
{
    {1.0, 2.0, 3.0, 4.0, 5.0},
    {2.0, 4.5, 5.0, 4.0, 6.0},
    {5.0, 3.0, 2.5, 2.0, 1.0},
    {0.5, 1.0, 9.0, 3.0, 8.0},
};

QVariantMap thresholdParameters(double threshold, CorrelationPolarity polarity)
{
    return
    {
        {u"minimumThreshold"_s, threshold},
        {u"correlationPolarity"_s, static_cast<int>(polarity)},
    };
}

QVariantMap allPairsParameters()
{
    return thresholdParameters(0.0, CorrelationPolarity::Both);
}

std::map<std::pair<int, int>, double> correlationsOf(const EdgeList& edgeList)
{
    std::map<std::pair<int, int>, double> correlations;

    for(const auto& edge : edgeList)
    {
        auto a = static_cast<int>(edge._source);
        auto b = static_cast<int>(edge._target);
        correlations[{std::min(a, b), std::max(a, b)}] = edge._weight;
    }

    return correlations;
}

QStringList pairsOf(const EdgeList& edgeList)
{
    const auto correlations = correlationsOf(edgeList);

    QStringList pairs;
    pairs.reserve(static_cast<qsizetype>(correlations.size()));

    for(const auto& [pair, r] : correlations)
        pairs.append(u"%1-%2"_s.arg(pair.first).arg(pair.second));

    return pairs;
}

std::unique_ptr<Normaliser> createNormaliser(const QString& name)
{
    if(name == u"MinMax"_s)             return std::make_unique<MinMaxNormaliser>();
    if(name == u"Mean"_s)               return std::make_unique<MeanNormaliser>();
    if(name == u"Standardisation"_s)    return std::make_unique<StandardisationNormaliser>();
    if(name == u"UnitScaling"_s)        return std::make_unique<UnitScalingNormaliser>();
    if(name == u"Quantile"_s)           return std::make_unique<QuantileNormaliser>();
    if(name == u"Softmax"_s)            return std::make_unique<SoftmaxNormaliser>();

    return nullptr;
}

bool fuzzyEqual(double a, double b)
{
    return std::abs(a - b) < 1e-6;
}
} // namespace

class TestCorrelation : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void continuousCorrelation_data()
    {
        QTest::addColumn<int>("type");
        QTest::addColumn<int>("a");
        QTest::addColumn<int>("b");
        QTest::addColumn<double>("expected");

        // Independently computed with numpy
        auto add = [](const char* name, CorrelationType type, const QList<double>& expected)
        {
            for(int i = 0, a = 0; a < continuousRows.size(); a++)
            {
                for(int b = a + 1; b < continuousRows.size(); b++)
                {
                    QTest::addRow("%s %d-%d", name, a, b) << static_cast<int>(type) <<
                        a << b << expected.at(i++);
                }
            }
        };

        add("Pearson", CorrelationType::Pearson,
            {0.799502686, -0.959403224, 0.678373852, -0.914772727, 0.767814038, -0.682737691});
        add("Spearman", CorrelationType::SpearmanRank,
            {0.7, -1.0, 0.7, -0.7, 0.8, -0.7});
        add("Euclidean", CorrelationType::EuclideanSimilarity,
            {0.222222222, 0.140780077, 0.127002455, 0.127892107, 0.143715584, 0.084792351});
        add("Cosine", CorrelationType::CosineSimilarity,
            {0.964836303, 0.631422302, 0.881983498, 0.738691661, 0.881351114, 0.501099759});
        add("Bicor", CorrelationType::Bicor,
            {0.724213450, -0.994384383, 0.660740496, -0.784287502, 0.785774973, -0.671857351});
    }

    void continuousCorrelation()
    {
        QFETCH(const int, type);
        QFETCH(const int, a);
        QFETCH(const int, b);
        QFETCH(const double, expected);

        auto correlation = ContinuousCorrelation::create(static_cast<CorrelationType>(type),
            CorrelationFilterType::Threshold);
        QVERIFY(correlation != nullptr);

        auto vectors = dataVectors<ContinuousDataVectors>(continuousRows);
        auto correlations = correlationsOf(correlation->edgeList(vectors, allPairsParameters()));

        QVERIFY(correlations.contains({a, b}));
        auto actual = correlations.at({a, b});
        QVERIFY2(fuzzyEqual(actual, expected), qPrintable(u"%1 != %2"_s.arg(actual).arg(expected)));
    }

    void discreteCorrelation_data()
    {
        QTest::addColumn<int>("type");
        QTest::addColumn<bool>("treatAsBinary");
        QTest::addColumn<QList<double>>("expected");

        // Expected values are for the pairs 0-1, 0-2 and 1-2; "", "0" and "false" are absences
        QTest::newRow("Jaccard") << static_cast<int>(CorrelationType::Jaccard) << false <<
            QList<double>{2.0 / 4.0, 1.0 / 4.0, 1.0 / 4.0};
        QTest::newRow("Jaccard binary") << static_cast<int>(CorrelationType::Jaccard) << true <<
            QList<double>{3.0 / 4.0, 1.0 / 4.0, 2.0 / 4.0};
        QTest::newRow("SMC") << static_cast<int>(CorrelationType::SMC) << false <<
            QList<double>{3.0 / 5.0, 2.0 / 5.0, 2.0 / 5.0};
        QTest::newRow("SMC binary") << static_cast<int>(CorrelationType::SMC) << true <<
            QList<double>{4.0 / 5.0, 2.0 / 5.0, 3.0 / 5.0};
    }

    void discreteCorrelation()
    {
        QFETCH(const int, type);
        QFETCH(const bool, treatAsBinary);
        QFETCH(const QList<double>, expected);

        const QList<QStringList> rows =
        {
            {u"a"_s, u"b"_s, u""_s, u"c"_s, u"0"_s},
            {u"a"_s, u"c"_s, u""_s, u"c"_s, u"x"_s},
            {u""_s,  u"b"_s, u""_s, u""_s,  u"x"_s},
        };

        auto correlation = DiscreteCorrelation::create(static_cast<CorrelationType>(type),
            CorrelationFilterType::Threshold);
        QVERIFY(correlation != nullptr);

        auto parameters = allPairsParameters();
        parameters.insert(u"treatAsBinary"_s, treatAsBinary);

        DiscreteDataVectors vectors;
        for(int i = 0; const auto& row : rows)
            vectors.emplace_back(std::vector<QString>{row.begin(), row.end()}, NodeId(i++));

        auto correlations = correlationsOf(correlation->edgeList(vectors, parameters));

        const QList<std::pair<int, int>> pairs = {{0, 1}, {0, 2}, {1, 2}};
        for(int i = 0; const auto& pair : pairs)
        {
            QVERIFY(correlations.contains(pair));
            auto actual = correlations.at(pair);
            QVERIFY2(fuzzyEqual(actual, expected.at(i)), qPrintable(u"%1-%2: %3 != %4"_s
                .arg(pair.first).arg(pair.second).arg(actual).arg(expected.at(i))));
            i++;
        }
    }

    void spearmanRankTies()
    {
        // Independently computed with numpy, giving tied values their mean rank
        const Rows rows =
        {
            {1.0, 2.0, 2.0, 3.0, 5.0},
            {4.0, 4.0, 1.0, 2.0, 2.0},
            {1.0, 1.0, 1.0, 2.0, 3.0},
        };

        auto correlation = ContinuousCorrelation::create(CorrelationType::SpearmanRank,
            CorrelationFilterType::Threshold);

        auto vectors = dataVectors<ContinuousDataVectors>(rows);
        auto correlations = correlationsOf(correlation->edgeList(vectors, allPairsParameters()));

        QVERIFY(fuzzyEqual(correlations[{0, 1}], -0.459627360));
        QVERIFY(fuzzyEqual(correlations[{0, 2}], 0.917662935));
        QVERIFY(fuzzyEqual(correlations[{1, 2}], -0.294627825));
    }

    void threshold_data()
    {
        QTest::addColumn<int>("polarity");
        QTest::addColumn<double>("threshold");
        QTest::addColumn<QStringList>("expected");

        // See continuousCorrelation_data for the Pearson values
        QTest::newRow("positive") << static_cast<int>(CorrelationPolarity::Positive) << 0.75 <<
            QStringList{u"0-1"_s, u"1-3"_s};
        QTest::newRow("negative") << static_cast<int>(CorrelationPolarity::Negative) << 0.75 <<
            QStringList{u"0-2"_s, u"1-2"_s};
        QTest::newRow("both") << static_cast<int>(CorrelationPolarity::Both) << 0.75 <<
            QStringList{u"0-1"_s, u"0-2"_s, u"1-2"_s, u"1-3"_s};
        QTest::newRow("none") << static_cast<int>(CorrelationPolarity::Positive) << 0.9 <<
            QStringList{};
    }

    void threshold()
    {
        QFETCH(const int, polarity);
        QFETCH(const double, threshold);
        QFETCH(const QStringList, expected);

        auto correlation = ContinuousCorrelation::create(CorrelationType::Pearson,
            CorrelationFilterType::Threshold);

        auto vectors = dataVectors<ContinuousDataVectors>(continuousRows);
        auto parameters = thresholdParameters(threshold, static_cast<CorrelationPolarity>(polarity));

        QCOMPARE(pairsOf(correlation->edgeList(vectors, parameters)), expected);
    }

    void knnProtoGraph_data()
    {
        QTest::addColumn<int>("k");
        QTest::addColumn<int>("polarity");
        QTest::addColumn<QStringList>("correlations");
        QTest::addColumn<QStringList>("expected");

        // Correlations are added in the order given; an edge is retained if
        // it is amongst the k strongest correlations of either of its nodes
        QTest::newRow("strongest") << 1 << static_cast<int>(CorrelationPolarity::Positive) <<
            QStringList{u"0-1:0.9"_s, u"0-2:0.5"_s, u"1-2:0.6"_s, u"0-3:0.1"_s, u"1-3:0.2"_s, u"2-3:0.95"_s} <<
            QStringList{u"0-1"_s, u"2-3"_s};
        QTest::newRow("below threshold") << 1 << static_cast<int>(CorrelationPolarity::Positive) <<
            QStringList{u"0-1:0.9"_s, u"0-2:-0.5"_s, u"1-2:-0.6"_s} <<
            QStringList{u"0-1"_s};
    }

    void knnProtoGraph()
    {
        QFETCH(const int, k);
        QFETCH(const int, polarity);
        QFETCH(const QStringList, correlations);
        QFETCH(const QStringList, expected);

        // The vectors' contents are irrelevant; only their identities are used
        auto vectors = dataVectors<ContinuousDataVectors>(Rows(5, {0.0}));

        auto parameters = thresholdParameters(0.0, static_cast<CorrelationPolarity>(polarity));
        parameters.insert(u"maximumK"_s, k);

        KnnProtoGraph<ContinuousDataVectors> protoGraph(vectors, parameters);

        for(const auto& correlation : correlations)
        {
            auto pairAndR = correlation.split(u':');
            auto pair = pairAndR.at(0).split(u'-');
            protoGraph.add(vectors.cbegin() + pair.at(0).toInt(),
                vectors.cbegin() + pair.at(1).toInt(), pairAndR.at(1).toDouble());
        }

        QStringList actual;
        actual.reserve(expected.size());
        for(const auto& relation : protoGraph)
        {
            auto a = static_cast<int>(relation._a - vectors.cbegin());
            auto b = static_cast<int>(relation._b - vectors.cbegin());
            actual.append(u"%1-%2"_s.arg(std::min(a, b)).arg(std::max(a, b)));
        }

        actual.sort();
        QCOMPARE(actual, expected);
    }

    void normalise_data()
    {
        QTest::addColumn<QString>("normaliser");
        QTest::addColumn<Rows>("input");
        QTest::addColumn<Rows>("expected");

        // Normalisation is per column; independently computed with numpy
        const Rows input = {{1.0, 10.0, 0.0}, {2.0, 20.0, 0.0}, {3.0, 40.0, 0.0}, {6.0, 30.0, 0.0}};

        QTest::newRow("MinMax") << u"MinMax"_s << input << Rows
        {
            {0.0, 0.0, 0.0}, {0.2, 0.333333333, 0.0}, {0.4, 1.0, 0.0}, {1.0, 0.666666667, 0.0}
        };

        QTest::newRow("Softmax") << u"Softmax"_s << input << Rows
        {
            {0.00626878689, 9.35719813e-14, 0.25}, {0.0170403295, 2.06106005e-09, 0.25},
            {0.046320418, 0.9999546, 0.25}, {0.930370466, 4.53978686e-05, 0.25}
        };

        QTest::newRow("Quantile") << u"Quantile"_s <<
            Rows{{5.0, 4.0, 3.0}, {2.0, 1.0, 4.0}, {3.0, 4.5, 6.0}, {4.0, 2.0, 8.0}} << Rows
        {
            {5.83333333, 4.66666667, 2.0}, {2.0, 2.0, 3.0},
            {3.0, 5.83333333, 4.66666667}, {4.66666667, 3.0, 5.83333333}
        };

        for(const auto* normaliser : {"MinMax", "Mean", "Standardisation", "Quantile", "Softmax"})
            QTest::addRow("%s empty", normaliser) << QString::fromLatin1(normaliser) << Rows{} << Rows{};
    }

    void normalise()
    {
        QFETCH(const QString, normaliser);
        QFETCH(const Rows, input);
        QFETCH(const Rows, expected);

        auto vectors = dataVectors<ContinuousDataVectors>(input);
        QVERIFY(createNormaliser(normaliser)->process(vectors));

        QCOMPARE(vectors.size(), static_cast<size_t>(expected.size()));

        for(size_t row = 0; row < vectors.size(); row++)
        {
            const auto& expectedRow = expected.at(static_cast<qsizetype>(row));

            for(size_t column = 0; column < vectors.at(row).size(); column++)
            {
                auto actual = vectors.at(row).valueAt(column);
                auto expectedValue = expectedRow.at(static_cast<qsizetype>(column));

                QVERIFY2(fuzzyEqual(actual, expectedValue), qPrintable(u"[%1][%2]: %3 != %4"_s
                    .arg(row).arg(column).arg(actual).arg(expectedValue)));
            }
        }
    }
};

QTEST_GUILESS_MAIN(TestCorrelation)
#include "tst_correlation.moc"
