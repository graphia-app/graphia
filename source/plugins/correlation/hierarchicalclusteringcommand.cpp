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

#include "hierarchicalclusteringcommand.h"

#include "correlationplugin.h"
#include "correlationdatavector.h"
#include "correlation.h"

#include <vector>
#include <limits>

#include <QObject>

class Unions
{
private:
    std::vector<size_t> _unions;

public:
    Unions(size_t size) : _unions(size)
    {
        std::iota(_unions.begin(), _unions.end(), 0);
    }

    size_t find(size_t i)
    {
        if(_unions.at(i) == i)
            return i;

        _unions[i] = find(_unions[i]);
        return _unions.at(i);
    }

    void join(size_t a, size_t b)
    {
        auto parentA = find(a);
        auto parentB = find(b);

        if(parentB > parentA)
            _unions[parentA] = parentB;
        else
            _unions[parentB] = parentA;
    }
};

struct Link
{
    size_t _index;
    size_t _pi;
};

struct EuclideanDistanceAlgorithm
{
    double evaluate(size_t size, const ContinuousDataVector* vectorA, const ContinuousDataVector* vectorB)
    {
        double sum = 0.0;

        for(size_t i = 0; i < size; i++)
        {
            auto diff = vectorA->valueAt(i) - vectorB->valueAt(i);
            auto diffSq = diff * diff;
            sum += diffSq;
        }

        return sum != 0.0 ? std::sqrt(sum) : 0.0;
    }
};

class EuclideanDistanceCorrelation : public CovarianceCorrelation<EuclideanDistanceAlgorithm>
{
public:
    QString name() const override { return {}; }
    QString description() const override { return {}; }
    QString attributeName() const override { return {}; }
    QString attributeDescription() const override { return {}; }
};

HierarchicalClusteringCommand::HierarchicalClusteringCommand(const std::vector<double>& data,
    size_t numColumns, size_t numRows, CorrelationPluginInstance& correlationPluginInstance) :
    _data(&data), _numColumns(numColumns), _numRows(numRows),
    _correlationPluginInstance(&correlationPluginInstance)
{}

bool HierarchicalClusteringCommand::execute()
{
    ContinuousDataVectors dataColumns;

    for(size_t column = 0; column < _numColumns; column++)
        dataColumns.emplace_back(*_data, column, _numColumns, _numRows);

    for(auto& dataColumn : dataColumns)
        dataColumn.update();

    EuclideanDistanceCorrelation correlation;

    setPhase(QObject::tr("Correlating"));
    auto matrix = correlation.matrix(dataColumns, this, this);

    if(cancelled())
        return false;

    setPhase(QObject::tr("Clustering"));

    std::vector<double> ls(dataColumns.size());
    std::vector<size_t> ps(dataColumns.size());
    std::vector<double> ms(dataColumns.size());

    ps[0] = 0;
    ls[0] = std::numeric_limits<double>::infinity();

    uint64_t progress = 0;
    uint64_t total = (dataColumns.size() * (dataColumns.size() - 1)) / 2;

    for(size_t i = 1; i < dataColumns.size(); i++)
    {
        ps[i] = i;
        ls[i] = std::numeric_limits<double>::infinity();

        for(size_t j = 0; j < i; j++)
            ms[j] = matrix.valueAt(i, j);

        for(size_t j = 0; j < i; j++)
        {
            auto pi = ps[j];

            if(ls[j] >= ms[j])
            {
                ms[pi] = std::min(ms[pi], ls[j]);
                ls[j] = ms[j];
                ps[j] = i;
            }
            else
                ms[pi] = std::min(ms[pi], ms[j]);
        }

        for(size_t j = 0; j < i; j++)
        {
            auto pi = ps[j];

            if(ls[j] >= ls[pi])
                ps[j] = i;

            setProgress(static_cast<int>((progress++ * 100) / total));
        }

        if(cancelled())
            return false;
    }

    setProgress(-1);

    std::vector<size_t> sortedIndices(dataColumns.size());
    std::iota(sortedIndices.begin(), sortedIndices.end(), 0);

    std::sort(sortedIndices.begin(), sortedIndices.end(),
        [&ls](auto a, auto b) { return ls.at(a) < ls.at(b); });

    std::vector<Link> links;
    links.reserve(dataColumns.size());

    Unions unions(dataColumns.size() * 2);

    // Generate linkage
    for(size_t i = 0; i < dataColumns.size() - 1; i++)
    {
        auto index = sortedIndices.at(i);
        auto pi = ps.at(index);

        auto rIndex = unions.find(index);
        auto rPi = unions.find(pi);

        links.push_back({rIndex, rPi});

        unions.join(index, dataColumns.size() + i);
        unions.join(pi, dataColumns.size() + i);
    }

    // Find leaves
    size_t orderingIndex = 0;
    size_t i = 0;
    std::vector<size_t> current(dataColumns.size());
    current[0] = (dataColumns.size() * 2) - 2;
    std::vector<bool> visited(dataColumns.size() * 2);
    std::vector<size_t> ordering(dataColumns.size());

    auto add = [&](size_t index)
    {
        if(visited.at(index))
            return false;

        visited[index] = true;
        if(index >= dataColumns.size())
        {
            current[++i] = index;
            return true;
        }

        ordering[index] = orderingIndex++;
        return false;
    };

    while(true)
    {
        const auto& link = links.at(current[i] - dataColumns.size());

        if(add(link._index) || add(link._pi))
            continue;

        if(i == 0)
            break;

        i--;
    }

    _correlationPluginInstance->setHcOrdering(ordering);

    return true;
}
