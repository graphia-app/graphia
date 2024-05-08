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

#ifndef CORRELATIONFILEPARSER_H
#define CORRELATIONFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/tabulardata.h"

#include "shared/utils/qmlenum.h"

#include "plugins/correlation/correlationdatavector.h"

#include <QString>
#include <QRect>

// Note: the ordering of these enums is important from a save
// file point of view; i.e. only append, don't reorder

DEFINE_QML_ENUM(
    Q_GADGET, ScalingType,
    None,
    Log2,
    Log10,
    AntiLog2,
    AntiLog10,
    ArcSin);

DEFINE_QML_ENUM(
    Q_GADGET, NormaliseType,
    None,
    MinMax,
    Quantile,
    Mean,
    Standarisation,
    UnitScaling,
    Softmax);

DEFINE_QML_ENUM(
    Q_GADGET, MissingDataType,
    Constant,
    ColumnAverage,
    RowInterpolation);

DEFINE_QML_ENUM(
    Q_GADGET, ClippingType,
    None,
    Constant,
    Winsorization);

class CorrelationPluginInstance;

class CorrelationFileParser : public IParser
{
private:
    CorrelationPluginInstance* _plugin;
    QString _urlTypeName;
    TabularData _tabularData;
    QRect _dataRect;

public:
    explicit CorrelationFileParser(CorrelationPluginInstance* plugin, const QString& urlTypeName,
        TabularData& tabularData, QRect dataRect);

    static double imputeValue(MissingDataType missingDataType, double replacementValue,
        const TabularData& tabularData, const QRect& dataRect, size_t columnIndex, size_t rowIndex);
    static void clipValues(ClippingType clippingType, double clippingValue, size_t width, std::vector<double>& data);
    static double scaleValue(ScalingType scalingType, double value,
        double epsilon = std::nextafter(0.0, 1.0));
    static void normalise(NormaliseType normaliseType,
        ContinuousDataVectors& dataRows,
        IParser* parser = nullptr);

    static double epsilonFor(const std::vector<double>& data);

    bool parse(const QUrl& fileUrl, IGraphModel* graphModel) override;
    QString log() const override;

    static bool canLoad(const QUrl&) { return true; }
};

#endif // CORRELATIONFILEPARSER_H
