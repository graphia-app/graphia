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

#include "tests/common/documenttest.h"
#include "tests/common/testenvironment.h"

#include "shared/loading/tabulardata.h"
#include "shared/utils/container.h"
#include "shared/ui/visualisations/ielementvisual.h"

#include <qcustomplot.h>
#include <qcustomplotcolorprovider.h>

#include <QApplication>
#include <QByteArray>
#include <QColor>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QFile>
#include <QHoverEvent>
#include <QList>
#include <QMetaObject>
#include <QMouseEvent>
#include <QPointF>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QSignalSpy>
#include <QSizeF>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QUrl>
#include <QVariantMap>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
// r2 is r1 doubled, r3 is r1 reversed, and r4 and r5 are like r1 and r3; the
// second row annotates the columns
const QByteArray Csv =
    "Name,Group,c1,c2,c3,c4,c5,c6\n"
    "Kind,,x,x,x,y,y,y\n"
    "r1,g1,1,2,3,4,5,6\n"
    "r2,g1,2,4,6,8,10,12\n"
    "r3,g2,6,5,4,3,2,1\n"
    "r4,g2,1,3,2,5,4,6\n"
    "r5,g2,6,4,5,2,3,1\n";

// The plot's options, as numbered in correlationplot.h
enum class ScaleType : uint8_t { Raw, Log, MeanCentre, UnitVariance, Pareto, ByAttribute, AntiLog };
enum class AveragingType : uint8_t { Individual, MeanLine, MedianLine, MeanHistogram, IQR };
enum class DispersionType : uint8_t { None, StdErr, StdDev };
enum class DispersionVisualType : uint8_t { Bars, Area };
enum class ColumnSortType : uint8_t { Natural, ColumnName, ColumnAnnotation, DataValue, HierarchicalClustering };
enum class PlotMode : uint8_t { Normal, ColumnAnnotationSelection, RowsOfInterestColumnSelection };

QString number(double value) { return QString::number(value, 'g', 6); }

// A box plot's whiskers, quartiles and median
QString box(const QCPStatisticalBoxData& data)
{
    return u"%1 %2 %3 %4 %5"_s.arg(number(data.minimum), number(data.lowerQuartile),
        number(data.median), number(data.upperQuartile), number(data.maximum));
}
} // namespace

// Stands in for the desktop, which is asked to show the images the plot saves
class UrlCatcher : public QObject
{
    Q_OBJECT

public:
    QList<QUrl> _urls;

public slots:
    void open(const QUrl& url) { _urls.append(url); }
};

// The plot in the correlation plugin's panel, created through QML as the panel does
class TestCorrelationPlot : public DocumentTest
{
    Q_OBJECT

    using DocumentTest::open;

    bool open(const QByteArray& csv = Csv, const QVariantMap& parameters = {})
    {
        // The plot uses the plugin, which goes with the Document
        destroyPlot();

        return open(write(u"data.csv"_s, csv), u"CorrelationCSV"_s, u"Correlation"_s, parameters);
    }

    QObject* plugin() { return _document->pluginInstance(); }

    std::unique_ptr<QQmlEngine> _engine;
    std::unique_ptr<QObject> _plot;
    QQuickItem* _item = nullptr;

    // The plot draws with a QCustomPlot, a widget of its own, which is what's checked
    QCustomPlot* _customPlot = nullptr;

    UrlCatcher _urlCatcher;

    void destroyPlot()
    {
        _plot.reset();
        _item = nullptr;
        _customPlot = nullptr;
    }

    // Changes the plot, then waits for it to be drawn again
    template<typename Fn>
    bool replot(const Fn& fn)
    {
        const QSignalSpy pixmapUpdated(_plot.get(), SIGNAL(pixmapUpdated()));
        fn();

        auto drawn = [&] { return !pixmapUpdated.isEmpty() && !_plot->property("busy").toBool(); };
        if(!QTest::qWaitFor(drawn, Timeout))
            return false;

        // It may be drawn again for something else, as when the nodes' colours change, and
        // then laid out differently, so wait for it to stop being drawn
        qsizetype count = 0;
        while(count != pixmapUpdated.count())
        {
            count = pixmapUpdated.count();
            QTest::qWait(50);

            if(!QTest::qWaitFor(drawn, Timeout))
                return false;
        }

        return true;
    }

    bool set(const char* name, const QVariant& value)
    {
        return replot([&] { _plot->setProperty(name, value); });
    }

    template<typename Enum>
    bool set(const char* name, Enum value) requires std::is_enum_v<Enum>
    {
        return set(name, static_cast<int>(value));
    }

    bool plotRows(const QList<int>& rows)
    {
        return replot([&] { QMetaObject::invokeMethod(_plot.get(), "setSelectedRows", Q_ARG(QList<int>, rows)); });
    }

    bool sortBy(ColumnSortType type, const QString& text = {})
    {
        return replot([&]
        {
            QMetaObject::invokeMethod(_plot.get(), "sortBy",
                Q_ARG(int, static_cast<int>(type)), Q_ARG(QString, text));
        });
    }

    template<typename... Args>
    bool invoke(const char* method, Args... args)
    {
        return replot([&] { QMetaObject::invokeMethod(_plot.get(), method, args...); });
    }

    // A plot of some rows, as in the plugin's panel when they're selected
    bool createPlot(const QList<int>& rows, const QSizeF& size = {800.0, 400.0})
    {
        const auto existingWidgets = QApplication::allWidgets();

        QQmlComponent component(_engine.get());
        component.setData("import Graphia.Plugins.Correlation\nCorrelationPlot {}", {});
        _plot.reset(component.create());
        _item = qobject_cast<QQuickItem*>(_plot.get());

        if(_item == nullptr)
        {
            qWarning() << component.errorString();
            return false;
        }

        const auto widgets = QApplication::allWidgets();
        for(auto* widget : widgets)
        {
            if(!existingWidgets.contains(widget))
                _customPlot = qobject_cast<QCustomPlot*>(widget);
        }

        if(_customPlot == nullptr)
            return false;

        _plot->setProperty("model", QVariant::fromValue(plugin()));
        _item->setSize(size);

        if(!plotRows(rows))
            return false;

        // The tooltip's marker is hidden when the cursor first moves, which draws the plot again,
        // without it being busy, so wait for that, rather than the plot being laid out during a click
        return hover({1.0, 1.0});
    }

    // The visible plottables of a type
    template<typename T = QCPAbstractPlottable>
    QList<T*> plottables()
    {
        QList<T*> list;
        for(int i = 0; i < _customPlot->plottableCount(); i++)
        {
            auto* plottable = dynamic_cast<T*>(_customPlot->plottable(i));
            if(plottable != nullptr && plottable->visible())
                list.append(plottable);
        }

        return list;
    }

    template<typename T = QCPAbstractPlottable>
    QStringList plottableNames()
    {
        QStringList names;
        const auto list = plottables<T>();
        for(const auto* plottable : list)
        {
            // The highlight of the chosen columns has no name
            if(!plottable->name().isEmpty())
                names.append(plottable->name());
        }

        names.sort();
        return names;
    }

    template<typename T = QCPAbstractPlottable>
    T* plottable(const QString& name)
    {
        const auto list = plottables<T>();
        auto it = std::ranges::find_if(list, [&name](const auto* p) { return p->name() == name; });
        return it != list.end() ? *it : nullptr;
    }

    // The values of a graph or bars, in column order
    template<typename T = QCPAbstractPlottable>
    QStringList values(const QString& name)
    {
        QStringList values;

        auto* p = plottable<T>(name);
        if(p == nullptr)
            return values;

        const auto* data = p->interface1D();
        values.reserve(data->dataCount());
        for(int i = 0; i < data->dataCount(); i++)
            values.append(number(data->dataMainValue(i)));

        return values;
    }

    // Each column's box, of every box plot
    QStringList boxes()
    {
        QStringList list;
        const auto boxPlots = plottables<QCPStatisticalBox>();
        for(const auto* boxPlot : boxPlots)
        {
            for(const auto& data : *boxPlot->data())
                list.append(box(data));
        }

        return list;
    }

    QCPAxisRect* dataAxisRect() { return _customPlot->axisRect(0); }
    QCPAxisRect* columnAnnotationsAxisRect() { return _customPlot->axisRect(1); }
    QCPRange valueRange() { return dataAxisRect()->axis(QCPAxis::atLeft)->range(); }

    // The pixel at which a column's value is drawn
    QPointF pixelFor(double column, double value)
    {
        return {dataAxisRect()->axis(QCPAxis::atBottom)->coordToPixel(column),
            dataAxisRect()->axis(QCPAxis::atLeft)->coordToPixel(value)};
    }

    // The pixel at the middle of a column, in an axis rectangle
    static QPointF columnPixel(QCPAxisRect* axisRect, double column)
    {
        return {axisRect->axis(QCPAxis::atBottom)->coordToPixel(column), static_cast<double>(axisRect->center().y())};
    }

    template<typename T>
    T* layoutElement()
    {
        const auto elements = _customPlot->plotLayout()->elements(true);
        for(auto* element : elements)
        {
            if(auto* t = dynamic_cast<T*>(element); t != nullptr)
                return t;
        }

        return nullptr;
    }

    QCPItemText* tooltip()
    {
        for(int i = 0; i < _customPlot->itemCount(); i++)
        {
            if(auto* text = dynamic_cast<QCPItemText*>(_customPlot->item(i)); text != nullptr)
                return text;
        }

        return nullptr;
    }

    // The names of the columns, in the order they're shown
    QStringList columnNames()
    {
        auto ticker = dataAxisRect()->axis(QCPAxis::atBottom)->ticker().dynamicCast<QCPAxisTickerText>();
        if(ticker == nullptr)
            return {};

        return ticker->ticks().values();
    }

    // The names the plot's left axis gives to the column annotations, from the bottom
    QStringList columnAnnotationLabels()
    {
        auto ticker = columnAnnotationsAxisRect()->axis(QCPAxis::atLeft)->ticker().dynamicCast<QCPAxisTickerText>();
        if(ticker == nullptr)
            return {};

        return ticker->ticks().values();
    }

    // Clicks the plot, as with a mouse
    void sendClick(const QPointF& position, Qt::KeyboardModifiers modifiers = Qt::NoModifier)
    {
        QMouseEvent press(QEvent::MouseButtonPress, position, position,
            Qt::LeftButton, Qt::LeftButton, modifiers);
        QCoreApplication::sendEvent(_item, &press);

        QMouseEvent release(QEvent::MouseButtonRelease, position, position,
            Qt::LeftButton, Qt::NoButton, modifiers);
        QCoreApplication::sendEvent(_item, &release);
    }

    bool click(const QPointF& position, Qt::KeyboardModifiers modifiers = Qt::NoModifier)
    {
        return replot([&] { sendClick(position, modifiers); });
    }

    bool hover(const QPointF& position)
    {
        return replot([&]
        {
            QHoverEvent event(QEvent::HoverMove, position, position, {});
            QCoreApplication::sendEvent(_item, &event);
        });
    }

    std::vector<int> selectedColumns() { return _plot->property("selectedColumns").value<std::vector<int>>(); }

    QColor nodeColor(const QString& name) { return graphModel()->nodeVisual(nodeId(name)).outerColor(); }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
        QVERIFY(createApplication());

        // The plugin's QML types are registered when it's loaded, by the Application
        _engine = std::make_unique<QQmlEngine>();

        QDesktopServices::setUrlHandler(u"file"_s, &_urlCatcher, "open");
    }

    void cleanup()
    {
        destroyPlot();
        _urlCatcher._urls.clear();

        closeDocument();
    }

    void cleanupTestCase()
    {
        QDesktopServices::unsetUrlHandler(u"file"_s);

        _engine.reset();
        destroyApplication();
    }

    // Each row is a line in the colour of its node
    void rows()
    {
        QVERIFY(open());
        QVERIFY(createPlot({0, 2}));

        QCOMPARE(_plot->property("selectedRows").value<QList<int>>(), (QList<int>{0, 2}));
        QCOMPARE(plottableNames<QCPGraph>(), (QStringList{u"r1"_s, u"r3"_s}));
        QCOMPARE(values(u"r1"_s), (QStringList{u"1"_s, u"2"_s, u"3"_s, u"4"_s, u"5"_s, u"6"_s}));
        QCOMPARE(values(u"r3"_s), (QStringList{u"6"_s, u"5"_s, u"4"_s, u"3"_s, u"2"_s, u"1"_s}));
        QCOMPARE(plottable(u"r1"_s)->pen().color(), nodeColor(u"r1"_s));

        // The value axis fits the values
        QCOMPARE(valueRange(), QCPRange(1.0, 6.0));

        QVERIFY(plotRows({1}));
        QCOMPARE(plottableNames<QCPGraph>(), QStringList{u"r2"_s});
        QCOMPARE(valueRange(), QCPRange(2.0, 12.0));

        QVERIFY(plotRows({}));
        QVERIFY(plottables<QCPGraph>().isEmpty());
    }

    void scaling_data()
    {
        QTest::addColumn<ScaleType>("scaleType");
        QTest::addColumn<QStringList>("values");

        QTest::newRow("Raw") << ScaleType::Raw << QStringList{u"1"_s, u"2"_s, u"3"_s, u"4"_s, u"5"_s, u"6"_s};

        // The log of the value plus half the smallest value, so that zeros can be scaled
        QTest::newRow("Log") << ScaleType::Log << QStringList{u"0.405465"_s, u"0.916291"_s,
            u"1.25276"_s, u"1.50408"_s, u"1.70475"_s, u"1.8718"_s};
        QTest::newRow("AntiLog") << ScaleType::AntiLog << QStringList{u"2.71828"_s, u"7.38906"_s,
            u"20.0855"_s, u"54.5982"_s, u"148.413"_s, u"403.429"_s};

        // Less the row's mean of 3.5, and divided by its standard deviation of 1.70783, or its root
        QTest::newRow("Mean centre") << ScaleType::MeanCentre << QStringList{u"-2.5"_s, u"-1.5"_s,
            u"-0.5"_s, u"0.5"_s, u"1.5"_s, u"2.5"_s};
        QTest::newRow("Unit variance") << ScaleType::UnitVariance << QStringList{u"-1.46385"_s,
            u"-0.87831"_s, u"-0.29277"_s, u"0.29277"_s, u"0.87831"_s, u"1.46385"_s};
        QTest::newRow("Pareto") << ScaleType::Pareto << QStringList{u"-1.91301"_s, u"-1.14781"_s,
            u"-0.382603"_s, u"0.382603"_s, u"1.14781"_s, u"1.91301"_s};
    }

    void scaling()
    {
        QFETCH(const ScaleType, scaleType);
        QFETCH(const QStringList, values);

        QVERIFY(open());
        QVERIFY(createPlot({0}));

        if(scaleType != ScaleType::Raw)
            QVERIFY(set("scaleType", scaleType));

        QCOMPARE(this->values(u"r1"_s), values);
        QCOMPARE(_plot->property("scaleType").toInt(), static_cast<int>(scaleType));
    }

    // Each row's values divided by one of its attributes, here its mean
    void scalingByAttribute()
    {
        QVERIFY(open());
        QVERIFY(createPlot({0}));

        QVERIFY(set("scaleByAttributeName", u"MeanDataValue"_s));
        QCOMPARE(_plot->property("scaleType").toInt(), static_cast<int>(ScaleType::ByAttribute));
        QCOMPARE(values(u"r1"_s), (QStringList{u"0.285714"_s, u"0.571429"_s, u"0.857143"_s,
            u"1.14286"_s, u"1.42857"_s, u"1.71429"_s}));

        // Without an attribute, there's no scaling
        QVERIFY(set("scaleByAttributeName", QString{}));
        QCOMPARE(_plot->property("scaleType").toInt(), static_cast<int>(ScaleType::Raw));
        QCOMPARE(values(u"r1"_s).constFirst(), u"1"_s);
    }

    void averaging_data()
    {
        QTest::addColumn<AveragingType>("averagingType");
        QTest::addColumn<QString>("name");
        QTest::addColumn<QStringList>("values");

        // Of r1, r2 and r3
        const QStringList means{u"3"_s, u"3.66667"_s, u"4.33333"_s, u"5"_s, u"5.66667"_s, u"6.33333"_s};

        QTest::newRow("Mean") << AveragingType::MeanLine << u"Mean average of selection"_s << means;
        QTest::newRow("Median") << AveragingType::MedianLine << u"Median average of selection"_s <<
            QStringList{u"2"_s, u"4"_s, u"4"_s, u"4"_s, u"5"_s, u"6"_s};
        QTest::newRow("Mean histogram") << AveragingType::MeanHistogram << u"Mean histogram of selection"_s << means;
    }

    void averaging()
    {
        QFETCH(const AveragingType, averagingType);
        QFETCH(const QString, name);
        QFETCH(const QStringList, values);

        QVERIFY(open());
        QVERIFY(createPlot({0, 1, 2}));
        QVERIFY(set("averagingType", averagingType));

        // Only the average is shown, in the colour the rows' nodes share
        QCOMPARE(plottableNames(), QStringList{name});
        QCOMPARE(this->values(name), values);

        auto color = plottable(name)->pen().color();
        if(averagingType == AveragingType::MeanHistogram)
            QCOMPARE(color, nodeColor(u"r1"_s).darker(150));
        else
            QCOMPARE(color, nodeColor(u"r1"_s));

        // Scaling that's relative to each row isn't applied to averages
        QVERIFY(set("averagingType", AveragingType::Individual));
        QVERIFY(set("scaleType", ScaleType::MeanCentre));
        QVERIFY(set("averagingType", averagingType));
        QCOMPARE(_plot->property("scaleType").toInt(), static_cast<int>(ScaleType::Raw));
        QCOMPARE(this->values(name), values);

        // ...but log scaling is
        QVERIFY(set("scaleType", ScaleType::Log));
        QCOMPARE(_plot->property("scaleType").toInt(), static_cast<int>(ScaleType::Log));
        QVERIFY(this->values(name) != values);
    }

    // An average for each value of an attribute
    void averagingByAttribute_data()
    {
        QTest::addColumn<AveragingType>("averagingType");
        QTest::addColumn<QString>("name");
        QTest::addColumn<QStringList>("g1");
        QTest::addColumn<QStringList>("g2");

        // g1 is r1 and r2, and g2 is r3, r4 and r5
        const QStringList g1{u"1.5"_s, u"3"_s, u"4.5"_s, u"6"_s, u"7.5"_s, u"9"_s};
        const QStringList g2Means{u"4.33333"_s, u"4"_s, u"3.66667"_s, u"3.33333"_s, u"3"_s, u"2.66667"_s};

        QTest::newRow("Mean") << AveragingType::MeanLine << u"Mean average"_s << g1 << g2Means;
        QTest::newRow("Median") << AveragingType::MedianLine << u"Median average"_s << g1 <<
            QStringList{u"6"_s, u"4"_s, u"4"_s, u"3"_s, u"3"_s, u"1"_s};
        QTest::newRow("Mean histogram") << AveragingType::MeanHistogram << u"Mean histogram"_s << g1 << g2Means;
    }

    void averagingByAttribute()
    {
        QFETCH(const AveragingType, averagingType);
        QFETCH(const QString, name);
        QFETCH(const QStringList, g1);
        QFETCH(const QStringList, g2);

        QVERIFY(open());
        QVERIFY(createPlot({0, 1, 2, 3, 4}));
        QVERIFY(set("averagingType", averagingType));
        QVERIFY(set("averagingAttributeName", u"Group"_s));

        const auto g1Name = u"%1 of Group: g1"_s.arg(name);
        const auto g2Name = u"%1 of Group: g2"_s.arg(name);
        QCOMPARE(plottableNames(), (QStringList{g1Name, g2Name}));
        QCOMPARE(values(g1Name), g1);
        QCOMPARE(values(g2Name), g2);

        if(averagingType == AveragingType::MeanHistogram)
        {
            // Side by side
            const auto bars = plottables<QCPBars>();
            QVERIFY(bars.at(0)->barsGroup() != nullptr);
            QCOMPARE(bars.at(0)->barsGroup(), bars.at(1)->barsGroup());
        }
    }

    // How much the averaged rows' values vary, about the mean, as bars or an area
    void dispersion_data()
    {
        QTest::addColumn<DispersionType>("dispersionType");
        QTest::addColumn<DispersionVisualType>("visualType");
        QTest::addColumn<QString>("name");
        QTest::addColumn<QStringList>("values");

        // Of r1 and r2, so the mean is 1.5 times r1, and the standard deviation is half of r1
        const QStringList stdDevs{u"0.5"_s, u"1"_s, u"1.5"_s, u"2"_s, u"2.5"_s, u"3"_s};
        const QStringList stdErrs{u"0.353553"_s, u"0.707107"_s, u"1.06066"_s, u"1.41421"_s, u"1.76777"_s, u"2.12132"_s};

        QTest::newRow("Standard deviation bars") << DispersionType::StdDev << DispersionVisualType::Bars <<
            u"Std Dev"_s << stdDevs;
        QTest::newRow("Standard error bars") << DispersionType::StdErr << DispersionVisualType::Bars <<
            u"Std Err"_s << stdErrs;

        // The tops of the areas; one standard deviation either side of the mean is r1 and r2
        QTest::newRow("Standard deviation area") << DispersionType::StdDev << DispersionVisualType::Area <<
            u"Std Dev"_s << QStringList{u"2"_s, u"4"_s, u"6"_s, u"8"_s, u"10"_s, u"12"_s};
        QTest::newRow("Standard error area") << DispersionType::StdErr << DispersionVisualType::Area <<
            u"Std Err"_s << QStringList{u"1.85355"_s, u"3.70711"_s, u"5.56066"_s,
            u"7.41421"_s, u"9.26777"_s, u"11.1213"_s};
    }

    void dispersion()
    {
        QFETCH(const DispersionType, dispersionType);
        QFETCH(const DispersionVisualType, visualType);
        QFETCH(const QString, name);
        QFETCH(const QStringList, values);

        QVERIFY(open());
        QVERIFY(createPlot({0, 1}));
        QVERIFY(set("averagingType", AveragingType::MeanLine));

        if(visualType != DispersionVisualType::Bars)
            QVERIFY(set("dispersionVisualType", visualType));

        QVERIFY(set("dispersionType", dispersionType));

        if(visualType == DispersionVisualType::Bars)
        {
            auto* errorBars = plottable<QCPErrorBars>(name);
            QVERIFY(errorBars != nullptr);

            QStringList errors;
            for(const auto& data : std::as_const(*errorBars->data()))
            {
                QCOMPARE(data.errorMinus, data.errorPlus);
                errors.append(number(data.errorPlus));
            }

            QCOMPARE(errors, values);
        }
        else
        {
            QCOMPARE(this->values(u"%1 Top"_s.arg(name)), values);

            // The bottom of the area is as far below the mean as the top is above it
            const auto means = this->values(u"Mean average of selection"_s);
            const auto tops = this->values(u"%1 Top"_s.arg(name));
            const auto bottoms = this->values(u"%1 Bottom"_s.arg(name));
            QCOMPARE(bottoms.size(), 6);
            for(qsizetype i = 0; i < bottoms.size(); i++)
            {
                const auto difference = (means.at(i).toDouble() - bottoms.at(i).toDouble()) -
                    (tops.at(i).toDouble() - means.at(i).toDouble());
                QVERIFY(std::abs(difference) < 0.0001);
            }
        }

        // The value axis includes the deviation from the largest mean, of 9
        const auto top = values.last().toDouble() + (visualType == DispersionVisualType::Bars ? 9.0 : 0.0);
        QCOMPARE(number(valueRange().upper), number(top));
    }

    // Individual rows and box plots have no dispersion
    void noDispersion_data()
    {
        QTest::addColumn<AveragingType>("averagingType");

        QTest::newRow("Individual") << AveragingType::Individual;
        QTest::newRow("IQR") << AveragingType::IQR;
    }

    void noDispersion()
    {
        QFETCH(const AveragingType, averagingType);

        QVERIFY(open());
        QVERIFY(createPlot({0, 1}));

        if(averagingType != AveragingType::Individual)
            QVERIFY(set("averagingType", averagingType));

        QVERIFY(set("dispersionType", DispersionType::StdDev));
        QVERIFY(plottables<QCPErrorBars>().isEmpty());
    }

    // A box plot of each column, with whiskers at the furthest values that aren't outliers
    void iqr_data()
    {
        QTest::addColumn<QList<int>>("rows");
        QTest::addColumn<QStringList>("boxes");

        QTest::newRow("Five rows") << QList<int>{0, 1, 2, 3, 4} << QStringList{u"1 1 2 6 6"_s,
            u"2 2.5 4 4.5 5"_s, u"2 2.5 4 5.5 6"_s, u"2 2.5 4 6.5 8"_s, u"2 2.5 4 7.5 10"_s, u"1 1 6 9 12"_s};
        QTest::newRow("Four rows") << QList<int>{0, 1, 2, 3} << QStringList{u"1 1 1.5 4 6"_s,
            u"2 2.5 3.5 4.5 5"_s, u"2 2.5 3.5 5 6"_s, u"3 3.5 4.5 6.5 8"_s, u"2 3 4.5 7.5 10"_s, u"1 3.5 6 9 12"_s};
        QTest::newRow("Three rows") << QList<int>{0, 1, 2} << QStringList{u"1 1 2 6 6"_s,
            u"2 2 4 5 5"_s, u"3 3 4 6 6"_s, u"3 3 4 8 8"_s, u"2 2 5 10 10"_s, u"1 1 6 12 12"_s};
        QTest::newRow("Two rows") << QList<int>{0, 1} << QStringList{u"1 1 1.5 2 2"_s,
            u"2 2 3 4 4"_s, u"3 3 4.5 6 6"_s, u"4 4 6 8 8"_s, u"5 5 7.5 10 10"_s, u"6 6 9 12 12"_s};
        QTest::newRow("One row") << QList<int>{2} << QStringList{u"6 6 6 6 6"_s,
            u"5 5 5 5 5"_s, u"4 4 4 4 4"_s, u"3 3 3 3 3"_s, u"2 2 2 2 2"_s, u"1 1 1 1 1"_s};
    }

    void iqr()
    {
        QFETCH(const QList<int>, rows);
        QFETCH(const QStringList, boxes);

        QVERIFY(open());
        QVERIFY(createPlot(rows));
        QVERIFY(set("averagingType", AveragingType::IQR));

        QCOMPARE(plottableNames<QCPStatisticalBox>(), QStringList(6, u"IQR"_s));
        QCOMPARE(this->boxes(), boxes);
    }

    // Values further than 1.5 times the interquartile range from the box are outliers
    void iqrOutliers()
    {
        QVERIFY(open(
            "Name,c1,c2\n"
            "a,1,0\nb,2,0\nc,3,0\nd,4,0\ne,5,0\nf,6,0\ng,7,0\nh,100,0\n"));
        QVERIFY(createPlot({0, 1, 2, 3, 4, 5, 6, 7}));
        QVERIFY(set("averagingType", AveragingType::IQR));

        auto outliers = [this]
        {
            QList<double> values;
            const auto& data = *plottables<QCPStatisticalBox>().constFirst()->data();
            values.reserve(data.at(0)->outliers.size());
            for(auto value : data.at(0)->outliers)
                values.append(value);

            return values;
        };

        // The upper quartile is 6.5 and the range is 4, so 100 is an outlier, and the whisker is at 7
        QCOMPARE(boxes().constFirst(), u"1 2.5 4.5 6.5 7"_s);
        QCOMPARE(outliers(), QList<double>{100.0});
        QCOMPARE(valueRange().upper, 100.0);

        QVERIFY(set("showIqrOutliers", false));
        QVERIFY(outliers().isEmpty());
        QCOMPARE(valueRange().upper, 7.0);

        // Other kinds of plot always show every value, so outliers are shown again after them
        QVERIFY(set("averagingType", AveragingType::MeanLine));
        QVERIFY(_plot->property("showIqrOutliers").toBool());
    }

    // The columns can be sorted, by several things at once
    void sorting()
    {
        QVERIFY(open());
        QVERIFY(createPlot({0}));

        auto sortOrder = [this]
        {
            const auto sortOrders = _plot->property("columnSortOrders").value<QVector<QVariantMap>>();
            QStringList list;
            list.reserve(sortOrders.size());
            for(const auto& columnSortOrder : sortOrders)
            {
                list.append(u"%1 %2 %3"_s.arg(QString::number(columnSortOrder.value(u"type"_s).toInt()),
                    columnSortOrder.value(u"text"_s).toString(),
                    columnSortOrder.value(u"order"_s).toInt() == Qt::AscendingOrder ? u"asc"_s : u"desc"_s));
            }

            return list;
        };

        // As r1 is 1 to 6, its values are the columns' numbers
        const QStringList natural{u"1"_s, u"2"_s, u"3"_s, u"4"_s, u"5"_s, u"6"_s};
        QCOMPARE(values(u"r1"_s), natural);

        // Sorting by the same thing again reverses the order
        QVERIFY(sortBy(ColumnSortType::ColumnName));
        QCOMPARE(values(u"r1"_s), natural);
        QVERIFY(sortBy(ColumnSortType::ColumnName));
        QCOMPARE(values(u"r1"_s), (QStringList{u"6"_s, u"5"_s, u"4"_s, u"3"_s, u"2"_s, u"1"_s}));
        QCOMPARE(sortOrder(), QStringList{u"1  desc"_s});

        // Another kind of sort replaces it
        QVERIFY(sortBy(ColumnSortType::Natural));
        QCOMPARE(values(u"r1"_s), natural);
        QCOMPARE(sortOrder(), QStringList{u"0  asc"_s});

        // Sorting by an annotation keeps what was sorted by before, for when its values are equal
        QVERIFY(sortBy(ColumnSortType::ColumnAnnotation, u"Kind"_s));
        QCOMPARE(values(u"r1"_s), natural);
        QVERIFY(sortBy(ColumnSortType::ColumnAnnotation, u"Kind"_s));
        QCOMPARE(values(u"r1"_s), (QStringList{u"4"_s, u"5"_s, u"6"_s, u"1"_s, u"2"_s, u"3"_s}));
        QCOMPARE(sortOrder(), (QStringList{u"2 Kind desc"_s, u"0  asc"_s}));

        // The annotation that's sorted by is marked
        QVERIFY(invoke("showColumnAnnotations", Q_ARG(QStringList, QStringList{u"Kind"_s})));
        QCOMPARE(columnAnnotationLabels(), QStringList{u"◁ Kind"_s});

        // Sorting by data value sorts by the rows' values, here r3's, which is 6 to 1
        QVERIFY(plotRows({2}));
        QVERIFY(!_plot->property("columnSortOrderCanBePinned").toBool());
        QVERIFY(sortBy(ColumnSortType::DataValue));
        QCOMPARE(values(u"r3"_s), natural);
        QVERIFY(_plot->property("columnSortOrderCanBePinned").toBool());

        // When it's pinned, the order stays the same when other rows are shown
        QVERIFY(_plot->setProperty("columnSortOrderPinned", true));
        QVERIFY(plotRows({0}));
        QCOMPARE(values(u"r1"_s), (QStringList{u"6"_s, u"5"_s, u"4"_s, u"3"_s, u"2"_s, u"1"_s}));

        QVERIFY(set("columnSortOrderPinned", false));
        QCOMPARE(values(u"r1"_s), natural);

        // Setting the sort orders, as when they're restored from a saved file
        QVERIFY(set("columnSortOrders", QVariant::fromValue(QVector<QVariantMap>{
            {{u"type"_s, static_cast<int>(ColumnSortType::ColumnName)}, {u"text"_s, QString{}},
            {u"order"_s, static_cast<int>(Qt::DescendingOrder)}}})));
        QCOMPARE(values(u"r1"_s), (QStringList{u"6"_s, u"5"_s, u"4"_s, u"3"_s, u"2"_s, u"1"_s}));
        QVERIFY(!_plot->property("columnSortOrderCanBePinned").toBool());
    }

    // The data value sorted by is the rows' average, when they're averaged
    void sortingByAverage_data()
    {
        QTest::addColumn<AveragingType>("averagingType");
        QTest::addColumn<QStringList>("values");

        // r3, r4 and r5's means are 4.33, 4, 3.67, 3.33, 3, 2.67, and their medians 6, 4, 4, 3, 3, 1
        QTest::newRow("Mean") << AveragingType::MeanLine <<
            QStringList{u"2.66667"_s, u"3"_s, u"3.33333"_s, u"3.66667"_s, u"4"_s, u"4.33333"_s};
        QTest::newRow("Median") << AveragingType::MedianLine <<
            QStringList{u"1"_s, u"3"_s, u"3"_s, u"4"_s, u"4"_s, u"6"_s};
    }

    void sortingByAverage()
    {
        QFETCH(const AveragingType, averagingType);
        QFETCH(const QStringList, values);

        QVERIFY(open());
        QVERIFY(createPlot({2, 3, 4}));
        QVERIFY(set("averagingType", averagingType));
        QVERIFY(sortBy(ColumnSortType::DataValue));

        QCOMPARE(this->values(plottableNames().constFirst()), values);
    }

    // The columns in the order hierarchical clustering finds
    void sortingByHierarchicalClustering()
    {
        // c1 and c3 are alike, as are c2 and c4
        QVERIFY(open(
            "Name,c1,c2,c3,c4\n"
            "r1,1,10,1.1,10\n"
            "r2,1,10,1,10\n"
            "r3,1,10,1,10.1\n"));

        const QSignalSpy complete(plugin(), SIGNAL(hierarchicalClusteringComplete()));
        QVERIFY(execute([this] { QMetaObject::invokeMethod(plugin(), "computeHierarchicalClustering"); }));
        QVERIFY(QTest::qWaitFor([&] { return !complete.isEmpty(); }, Timeout));

        QVERIFY(createPlot({0}));
        QCOMPARE(columnNames(), (QStringList{u"c1"_s, u"c2"_s, u"c3"_s, u"c4"_s}));

        QVERIFY(sortBy(ColumnSortType::HierarchicalClustering));
        const QStringList order{u"c1"_s, u"c3"_s, u"c2"_s, u"c4"_s};
        QCOMPARE(columnNames(), order);

        QVERIFY(sortBy(ColumnSortType::HierarchicalClustering));
        QStringList reversed = order;
        std::ranges::reverse(reversed);
        QCOMPARE(columnNames(), reversed);
    }

    // Annotations of numbers are sorted as numbers, rather than as text, where 1.5 is before 1.25
    void sortingByNumericAnnotation()
    {
        QVERIFY(open());

        auto data = std::make_shared<TabularData>();
        const QList<QStringList> rows
        {
            {u"Column"_s, u"c1"_s, u"c2"_s, u"c3"_s, u"c4"_s, u"c5"_s, u"c6"_s},
            {u"Batch"_s,  u"1.5"_s, u"1.25"_s, u"10"_s, u"-3"_s, u"0"_s, u"20"_s},
        };

        data->reserve(7, 2);
        for(qsizetype row = 0; row < rows.size(); row++)
        {
            for(qsizetype column = 0; column < rows.at(row).size(); column++)
                data->setValueAt(static_cast<size_t>(column), static_cast<size_t>(row), rows.at(row).at(column));
        }

        QVERIFY(execute([&]
        {
            QMetaObject::invokeMethod(plugin(), "importAnnotationsFromTable",
                Q_ARG(std::shared_ptr<TabularData>, data), Q_ARG(int, 0),
                Q_ARG(std::vector<int>, std::vector<int>{1}), Q_ARG(bool, false));
        }));

        QVERIFY(createPlot({0}));
        QVERIFY(sortBy(ColumnSortType::ColumnAnnotation, u"Batch"_s));
        QCOMPARE(values(u"r1"_s), (QStringList{u"4"_s, u"5"_s, u"2"_s, u"1"_s, u"3"_s, u"6"_s}));
    }

    // The columns can be grouped by their annotations, with a box plot of each group
    void grouping()
    {
        QVERIFY(open());
        QVERIFY(createPlot({0, 1}));

        const auto minimumHeight = _plot->property("minimumHeight").toDouble();
        const QSignalSpy visibleColumnAnnotationNamesChanged(_plot.get(), SIGNAL(visibleColumnAnnotationNamesChanged()));

        QVERIFY(invoke("showAllColumnAnnotations"));
        QCOMPARE(_plot->property("visibleColumnAnnotationNames").toStringList(), QStringList{u"Kind"_s});
        QCOMPARE(visibleColumnAnnotationNamesChanged.count(), 1);
        QVERIFY(_plot->property("minimumHeight").toDouble() > minimumHeight);
        QCOMPARE(columnAnnotationLabels(), QStringList{u"Kind"_s});

        // x is c1 to c3, and y is c4 to c6, of r1 and r2
        QVERIFY(set("groupByAnnotation", true));
        QCOMPARE(boxes(), (QStringList{u"1 2 2.5 4 6"_s, u"4 5 7 10 12"_s}));
        QCOMPARE(plottableNames<QCPStatisticalBox>(), (QStringList{u"IQR"_s, u"IQR"_s}));
        QVERIFY(plottables<QCPGraph>().isEmpty());

        // The boxes are coloured as the annotation's values are
        QVERIFY(set("colorGroupByAnnotationName", u"Kind"_s));
        QCOMPARE(plottableNames<QCPStatisticalBox>(), (QStringList{u"IQR of x"_s, u"IQR of y"_s}));
        const auto boxPlots = plottables<QCPStatisticalBox>();
        QVERIFY(boxPlots.at(0)->brush().color() != boxPlots.at(1)->brush().color());

        // The outliers can be hidden
        QVERIFY(set("showIqrOutliers", false));
        QVERIFY(set("groupByAnnotation", false));
        QVERIFY(_plot->property("showIqrOutliers").toBool());
        QCOMPARE(plottableNames<QCPGraph>(), (QStringList{u"r1"_s, u"r2"_s}));

        // Grouping is turned off when there are no annotations to group by
        QVERIFY(set("groupByAnnotation", true));
        QVERIFY(invoke("hideColumnAnnotations", Q_ARG(QStringList, QStringList{u"Kind"_s})));
        QVERIFY(_plot->property("visibleColumnAnnotationNames").toStringList().isEmpty());
        QCOMPARE(_plot->property("minimumHeight").toDouble(), minimumHeight);
        QVERIFY(set("plotMode", PlotMode::ColumnAnnotationSelection));
        QVERIFY(_plot->property("groupByAnnotation").toBool());
        QVERIFY(set("plotMode", PlotMode::Normal));
        QVERIFY(!_plot->property("groupByAnnotation").toBool());

        QVERIFY(invoke("showColumnAnnotations", Q_ARG(QStringList, QStringList{u"Kind"_s})));
        QVERIFY(invoke("hideAllColumnAnnotations"));
        QVERIFY(_plot->property("visibleColumnAnnotationNames").toStringList().isEmpty());

        // As when the annotations have changed
        QVERIFY(invoke("refreshColumnAnnotations"));
    }

    // Choosing which annotations are shown, from all of them, with the mouse
    void selectingColumnAnnotations()
    {
        QVERIFY(open());
        QVERIFY(createPlot({0}));

        const auto minimumHeight = _plot->property("minimumHeight").toDouble();

        // Every annotation is shown when choosing, with whether it's chosen
        QVERIFY(set("plotMode", PlotMode::ColumnAnnotationSelection));
        QCOMPARE(_plot->property("plotMode").toInt(), static_cast<int>(PlotMode::ColumnAnnotationSelection));
        QVERIFY(_plot->property("minimumHeight").toDouble() > minimumHeight);
        QCOMPARE(columnAnnotationLabels(), QStringList{u"Kind ☐"_s});

        // Clicking on an annotation that isn't shown shows it
        QVERIFY(click(columnPixel(columnAnnotationsAxisRect(), 0.0)));
        QCOMPARE(_plot->property("visibleColumnAnnotationNames").toStringList(), QStringList{u"Kind"_s});
        QCOMPARE(columnAnnotationLabels(), QStringList{u"Kind ☑"_s});

        // ...and clicking on it again sorts by it
        QVERIFY(click(columnPixel(columnAnnotationsAxisRect(), 0.0)));
        QCOMPARE(columnAnnotationLabels(), QStringList{u"▷ Kind ☑"_s});

        // Clicking on its name hides it
        auto namePixel = columnPixel(columnAnnotationsAxisRect(), 0.0);
        namePixel.setX(columnAnnotationsAxisRect()->left() - 5.0);
        QVERIFY(click(namePixel));
        QVERIFY(_plot->property("visibleColumnAnnotationNames").toStringList().isEmpty());
        QVERIFY(click(namePixel));
        QCOMPARE(_plot->property("visibleColumnAnnotationNames").toStringList(), QStringList{u"Kind"_s});

        // When not choosing, only the shown annotations are, and clicking on a name sorts by it
        QVERIFY(set("plotMode", PlotMode::Normal));
        QCOMPARE(columnAnnotationLabels(), QStringList{u"▷ Kind"_s});
        namePixel = columnPixel(columnAnnotationsAxisRect(), 0.0);
        namePixel.setX(columnAnnotationsAxisRect()->left() - 5.0);
        QVERIFY(click(namePixel));
        QCOMPARE(columnAnnotationLabels(), QStringList{u"◁ Kind"_s});
        QCOMPARE(values(u"r1"_s), (QStringList{u"4"_s, u"5"_s, u"6"_s, u"1"_s, u"2"_s, u"3"_s}));
    }

    // Choosing columns, with the mouse, to find the rows of interest in
    void selectingColumns()
    {
        QVERIFY(open());
        QVERIFY(createPlot({0}));

        // Clicking does nothing, normally
        sendClick(pixelFor(2.0, 3.0));
        QVERIFY(selectedColumns().empty());

        QVERIFY(set("plotMode", PlotMode::RowsOfInterestColumnSelection));

        QVERIFY(click(pixelFor(2.0, 3.0)));
        QCOMPARE(selectedColumns(), std::vector<int>{2});

        // With control, columns are added, or removed
        QVERIFY(click(pixelFor(4.0, 3.0), Qt::ControlModifier));
        QCOMPARE(selectedColumns(), (std::vector<int>{2, 4}));
        QVERIFY(click(pixelFor(2.0, 3.0), Qt::ControlModifier));
        QCOMPARE(selectedColumns(), std::vector<int>{4});

        // Without, they're replaced
        QVERIFY(click(pixelFor(0.0, 3.0)));
        QCOMPARE(selectedColumns(), std::vector<int>{0});

        // The columns are those that are shown there, however they're sorted
        QVERIFY(sortBy(ColumnSortType::ColumnName));
        QVERIFY(sortBy(ColumnSortType::ColumnName));
        QVERIFY(click(pixelFor(0.0, 3.0)));
        QCOMPARE(selectedColumns(), std::vector<int>{5});

        // Clicking on an annotation chooses every column with the same value
        QVERIFY(sortBy(ColumnSortType::Natural));
        QVERIFY(invoke("showColumnAnnotations", Q_ARG(QStringList, QStringList{u"Kind"_s})));
        QVERIFY(click(columnPixel(columnAnnotationsAxisRect(), 4.0)));
        QCOMPARE(selectedColumns(), (std::vector<int>{3, 4, 5}));
        QVERIFY(click(columnPixel(columnAnnotationsAxisRect(), 0.0), Qt::ControlModifier));
        QCOMPARE(selectedColumns(), (std::vector<int>{0, 1, 2, 3, 4, 5}));
        QVERIFY(click(columnPixel(columnAnnotationsAxisRect(), 5.0), Qt::ControlModifier));
        QCOMPARE(selectedColumns(), (std::vector<int>{0, 1, 2}));

        // ...and of the groups, when grouped
        QVERIFY(set("groupByAnnotation", true));
        QVERIFY(click(columnPixel(columnAnnotationsAxisRect(), 1.0)));
        QCOMPARE(selectedColumns(), (std::vector<int>{3, 4, 5}));

        // The choice is forgotten when no longer choosing
        const QSignalSpy selectedColumnsChanged(_plot.get(), SIGNAL(selectedColumnsChanged()));
        QVERIFY(set("plotMode", PlotMode::Normal));
        QVERIFY(selectedColumns().empty());
        QCOMPARE(selectedColumnsChanged.count(), 1);
    }

    // Clicking the columns' names sorts by them, and clicking left of them restores the order
    void clickingColumnNames()
    {
        QVERIFY(open());
        QVERIFY(createPlot({0}));

        auto namesPixel = pixelFor(2.0, 0.0);
        namesPixel.setY(dataAxisRect()->bottom() + 5.0);

        QVERIFY(click(namesPixel));
        QVERIFY(click(namesPixel));
        QCOMPARE(values(u"r1"_s), (QStringList{u"6"_s, u"5"_s, u"4"_s, u"3"_s, u"2"_s, u"1"_s}));

        namesPixel.setX(dataAxisRect()->left() - 5.0);
        QVERIFY(click(namesPixel));
        QCOMPARE(values(u"r1"_s), (QStringList{u"1"_s, u"2"_s, u"3"_s, u"4"_s, u"5"_s, u"6"_s}));

        // In the mode for choosing columns, clicking a name chooses its column
        QVERIFY(set("plotMode", PlotMode::RowsOfInterestColumnSelection));
        namesPixel.setX(pixelFor(3.0, 0.0).x());
        QVERIFY(click(namesPixel));
        QCOMPARE(selectedColumns(), std::vector<int>{3});
    }

    // Hovering over a value shows what it is
    void tooltips()
    {
        QVERIFY(open());
        QVERIFY(createPlot({0}));

        QVERIFY(tooltip() != nullptr);
        QVERIFY(!tooltip()->visible());

        QVERIFY(hover(pixelFor(2.0, 3.0)));
        QVERIFY(tooltip()->visible());
        QCOMPARE(tooltip()->text(), u"r1, c3: 3"_s);

        // Somewhere with nothing
        QVERIFY(hover(pixelFor(2.0, 6.0)));
        QVERIFY(!tooltip()->visible());

        // Near enough, within 24 pixels
        QVERIFY(hover(pixelFor(4.0, 5.0) + QPointF{0.0, 10.0}));
        QCOMPARE(tooltip()->text(), u"r1, c5: 5"_s);

        QVERIFY(replot([this]
        {
            QHoverEvent event(QEvent::HoverLeave, {}, {}, {});
            QCoreApplication::sendEvent(_item, &event);
        }));
        QVERIFY(!tooltip()->visible());

        // An annotation's value
        QVERIFY(invoke("showColumnAnnotations", Q_ARG(QStringList, QStringList{u"Kind"_s})));
        QVERIFY(hover(columnPixel(columnAnnotationsAxisRect(), 4.0)));
        QCOMPARE(tooltip()->text(), u"y"_s);

        // When grouped, how many columns are in a group
        QVERIFY(set("groupByAnnotation", true));
        const auto& data = *plottables<QCPStatisticalBox>().constFirst()->data();
        QVERIFY(hover(pixelFor(0.0, data.at(0)->median)));
        QCOMPARE(tooltip()->text(), u"IQR, 3 columns: 2"_s);

        // A bar
        QVERIFY(set("groupByAnnotation", false));
        QVERIFY(set("averagingType", AveragingType::MeanHistogram));
        QVERIFY(hover(pixelFor(2.0, 1.5)));
        QCOMPARE(tooltip()->text(), u"Mean histogram of selection, c3: 3"_s);
    }

    // The value axis can be zoomed with the mouse wheel, and panned by dragging
    void zoomAndPan()
    {
        QVERIFY(open());
        QVERIFY(createPlot({0}));
        QVERIFY(!_plot->property("zoomed").toBool());

        const auto centre = dataAxisRect()->center();
        QVERIFY(replot([&]
        {
            QWheelEvent event(centre, centre, {}, {0, 120}, Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
            QCoreApplication::sendEvent(_item, &event);
        }));

        QVERIFY(QTest::qWaitFor([this] { return _plot->property("zoomed").toBool(); }, Timeout));
        const auto zoomedRange = valueRange();
        QVERIFY(zoomedRange.size() < 5.0);
        QVERIFY(zoomedRange.lower > 1.0 && zoomedRange.upper < 6.0);

        // Dragging down moves the values down, so higher ones are shown
        QVERIFY(replot([&]
        {
            QMouseEvent press(QEvent::MouseButtonPress, centre, centre, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            QCoreApplication::sendEvent(_item, &press);

            const QPointF to = centre + QPointF{0.0, 20.0};
            QMouseEvent move(QEvent::MouseMove, to, to, Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
            QCoreApplication::sendEvent(_item, &move);
        }));

        QVERIFY(valueRange().lower > zoomedRange.lower);
        QCOMPARE(number(valueRange().size()), number(zoomedRange.size()));

        QVERIFY(invoke("resetZoom"));
        QVERIFY(QTest::qWaitFor([this] { return !_plot->property("zoomed").toBool(); }, Timeout));
        QCOMPARE(valueRange(), QCPRange(1.0, 6.0));
    }

    // The legend lists the rows
    void legend()
    {
        QVERIFY(open());
        QVERIFY(createPlot({0, 2}));
        QVERIFY(layoutElement<QCPLegend>() == nullptr);

        QVERIFY(set("showLegend", true));
        auto* legend = layoutElement<QCPLegend>();
        QVERIFY(legend != nullptr);

        QStringList names;
        for(int i = 0; i < legend->itemCount(); i++)
        {
            auto* item = qobject_cast<QCPPlottableLegendItem*>(legend->item(i));
            QVERIFY(item != nullptr);
            names.append(item->plottable()->name());
        }

        QCOMPARE(names, (QStringList{u"r1"_s, u"r3"_s}));

        QVERIFY(set("showLegend", false));
        QVERIFY(layoutElement<QCPLegend>() == nullptr);
    }

    // When there are more rows than fit, the legend says how many it leaves out
    void legendTruncated()
    {
        QByteArray csv = "Name,c1,c2,c3,c4\n";
        for(int i = 0; i < 30; i++)
            csv.append(u"r%1,1,%2,3,4\n"_s.arg(i, 2, 10, QChar(u'0')).arg(2 + (i * 0.01)).toUtf8());

        QVERIFY(open(csv));

        QList<int> rows(30);
        u::iota(rows, 0);
        QVERIFY(createPlot(rows, {800.0, 200.0}));
        QVERIFY(set("showLegend", true));

        auto* legend = layoutElement<QCPLegend>();
        QVERIFY(legend != nullptr);

        QStringList names;
        names.reserve(legend->itemCount());
        for(int i = 0; i < legend->itemCount(); i++)
        {
            auto* item = qobject_cast<QCPPlottableLegendItem*>(legend->item(i));
            if(item != nullptr)
                names.append(item->plottable()->name());
        }

        QVERIFY(!names.isEmpty());
        QVERIFY(names.size() < 30);

        // The first rows by name, then the number of the rest
        QStringList expectedNames;
        expectedNames.reserve(names.size());
        for(qsizetype i = 0; i < names.size(); i++)
            expectedNames.append(u"r%1"_s.arg(i, 2, 10, QChar(u'0')));
        QCOMPARE(names, expectedNames);

        QString moreText;
        for(int i = 0; i < legend->elementCount(); i++)
        {
            if(auto* text = dynamic_cast<QCPTextElement*>(legend->elementAt(i)); text != nullptr)
                moreText = text->text();
        }

        QCOMPARE(moreText, u"…and %1 more"_s.arg(30 - names.size()));
    }

    // The labels of the axes
    void axisLabels()
    {
        QVERIFY(open());
        QVERIFY(createPlot({0}));
        QVERIFY(layoutElement<QCPTextElement>() == nullptr);

        QVERIFY(set("xAxisLabel", u"Columns"_s));
        QCOMPARE(layoutElement<QCPTextElement>()->text(), u"Columns"_s);

        QVERIFY(set("yAxisLabel", u"Values"_s));
        QCOMPARE(dataAxisRect()->axis(QCPAxis::atLeft)->label(), u"Values"_s);

        // The column names are shown, unless there's no room for them
        QVERIFY(dataAxisRect()->axis(QCPAxis::atBottom)->tickLabels());
        QVERIFY(set("elideLabelWidth", 0));
        QVERIFY(!dataAxisRect()->axis(QCPAxis::atBottom)->tickLabels());
        QCOMPARE(layoutElement<QCPTextElement>()->text(), u"Columns (Resize To Expose Column Names)"_s);

        QVERIFY(invoke("showColumnAnnotations", Q_ARG(QStringList, QStringList{u"Kind"_s})));
        QCOMPARE(layoutElement<QCPTextElement>()->text(), u"Columns (Resize To Expose Column Information)"_s);

        QVERIFY(set("xAxisLabel", QString{}));
        QCOMPARE(layoutElement<QCPTextElement>()->text(), u"Resize To Expose Column Information"_s);

        QVERIFY(invoke("hideAllColumnAnnotations"));
        QVERIFY(set("showColumnNames", false));
        QVERIFY(layoutElement<QCPTextElement>() == nullptr);
    }

    // The value axis can include zero, however far the values are from it
    void includeYZero()
    {
        QVERIFY(open());
        QVERIFY(createPlot({0}));
        QCOMPARE(valueRange(), QCPRange(1.0, 6.0));

        QVERIFY(set("includeYZero", true));
        QCOMPARE(valueRange(), QCPRange(0.0, 6.0));

        // Negative values, after scaling by the log of values less than one
        QVERIFY(open("Name,c1,c2\na,0.1,0.2\nb,0.2,0.1\n"));
        QVERIFY(createPlot({0}));
        QVERIFY(set("scaleType", ScaleType::Log));
        QVERIFY(valueRange().upper < 0.0);

        QVERIFY(set("includeYZero", true));
        QCOMPARE(valueRange().upper, 0.0);
    }

    // How much of the plot can be seen across, depending on how wide the columns need to be
    void horizontalScrolling()
    {
        QVERIFY(open());
        QVERIFY(createPlot({0}, {70.0, 400.0}));

        auto visibleFraction = [this] { return _plot->property("visibleHorizontalFraction").toDouble(); };

        // Each column needs to be as wide as a line of text, for its name
        QVERIFY(visibleFraction() < 1.0);
        QVERIFY(!_plot->property("isWide").toBool());

        QVERIFY(set("horizontalScrollPosition", 1.0));
        QCOMPARE(_plot->property("horizontalScrollPosition").toDouble(), 1.0);

        // The position is between 0 and 1
        QVERIFY(set("horizontalScrollPosition", -1.0));
        QCOMPARE(_plot->property("horizontalScrollPosition").toDouble(), 0.0);

        // Without names, columns can be a pixel wide
        QVERIFY(set("showColumnNames", false));
        QCOMPARE(visibleFraction(), 1.0);

        // ...but not when they show the deviation
        QVERIFY(set("dispersionType", DispersionType::StdDev));
        QVERIFY(visibleFraction() < 1.0);

        // Unless they're all to be shown
        QVERIFY(set("showAllColumns", true));
        QCOMPARE(visibleFraction(), 1.0);

        QVERIFY(set("rightPadding", 10));
        QVERIFY(set("bottomPadding", 10));
        QVERIFY(set("showGridLines", false));
    }

    // The average of rows whose nodes are different colours is drawn in black, or white
    void colors()
    {
        QVERIFY(open());
        QVERIFY(createPlot({0, 2}));
        QVERIFY(set("averagingType", AveragingType::MeanLine));

        const auto name = u"Mean average of selection"_s;
        QCOMPARE(plottable(name)->pen().color(), nodeColor(u"r1"_s));

        // The plot is redrawn when the colours change
        QVERIFY(replot([this] { execute([this] { _document->update({}, {u"Group Colour"_s}); }); }));
        QVERIFY(nodeColor(u"r1"_s) != nodeColor(u"r3"_s));
        QCOMPARE(plottable(name)->pen().color(), QCustomPlotColorProvider::penColor());

        QVERIFY(plotRows({0, 1}));
        QCOMPARE(plottable(name)->pen().color(), nodeColor(u"r1"_s));
    }

    // Discrete data is shown as how many times each value occurs in each column
    void discrete()
    {
        QVERIFY(open(
            "Name,a,b,c\n"
            "r1,x,y,z\n"
            "r2,x,y,y\n"
            "r3,z,z,z\n"
            "r4,x,y,z\n",
        {
            {u"correlationDataType"_s, u"Discrete"_s},
            {u"discreteCorrelationType"_s, u"SMC"_s},
            {u"minimumThreshold"_s, 0.5},
            {u"initialThreshold"_s, 0.5},
        }));
        QVERIFY(createPlot({0, 1, 2, 3}));

        auto barsList = [this]
        {
            QStringList list;
            const auto bars = plottables<QCPBars>();
            for(auto* b : bars)
            {
                const auto* data = b->interface1D();
                list.append(u"%1 %2 %3"_s.arg(data->dataMainKey(0)).arg(b->name()).arg(data->dataMainValue(0)));
            }

            list.sort();
            return list;
        };

        QCOMPARE(barsList(), (QStringList{u"0 x 3"_s, u"0 z 1"_s, u"1 y 3"_s, u"1 z 1"_s, u"2 y 1"_s, u"2 z 3"_s}));
        QCOMPARE(valueRange(), QCPRange(0.0, 4.0));

        // The values are stacked
        auto* z = plottable<QCPBars>(u"z"_s);
        QVERIFY(z->barAbove() != nullptr || z->barBelow() != nullptr);

        QVERIFY(plotRows({2}));
        QCOMPARE(barsList(), (QStringList{u"0 z 1"_s, u"1 z 1"_s, u"2 z 1"_s}));

        QVERIFY(hover(pixelFor(0.0, 0.5)));
        QCOMPARE(tooltip()->text(), u"z, a: 1"_s);
    }

    // Discrete columns sorted by value are sorted by how many of the rows plotted have a value
    void discreteSortedByValue()
    {
        // a has 3 values, b 1 and c 2; of r1 and r2, a has 1, b none and c 2; the data rectangle
        // is given, as with so many missing values, the one found is smaller
        QVERIFY(open(
            "Name,a,b,c\n"
            "r1,x,,z\n"
            "r2,,,z\n"
            "r3,x,y,\n"
            "r4,x,,\n",
        {
            {u"dataRect"_s, QVariantMap{{u"x"_s, 1}, {u"y"_s, 1}, {u"width"_s, 3}, {u"height"_s, 4}}},
            {u"correlationDataType"_s, u"Discrete"_s},
            {u"discreteCorrelationType"_s, u"SMC"_s},
            {u"minimumThreshold"_s, 0.0},
            {u"initialThreshold"_s, 0.0},
        }));
        QVERIFY(createPlot({0, 1, 2, 3}));
        QCOMPARE(columnNames(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));

        QVERIFY(sortBy(ColumnSortType::DataValue));
        QCOMPARE(columnNames(), (QStringList{u"b"_s, u"c"_s, u"a"_s}));
        QVERIFY(sortBy(ColumnSortType::DataValue));
        QCOMPARE(columnNames(), (QStringList{u"a"_s, u"c"_s, u"b"_s}));

        QVERIFY(plotRows({0, 1}));
        QCOMPARE(columnNames(), (QStringList{u"c"_s, u"a"_s, u"b"_s}));
    }

    // When there are many values, a column is one bar
    void manyDiscreteValues()
    {
        QByteArray csv = "Name,a,b\n";
        for(int i = 0; i < 16; i++)
            csv.append(u"r%1,v%2,w\n"_s.arg(i).arg(i, 2, 10, QChar(u'0')).toUtf8());

        QVERIFY(open(csv,
        {
            {u"correlationDataType"_s, u"Discrete"_s},
            {u"discreteCorrelationType"_s, u"SMC"_s},
            {u"minimumThreshold"_s, 0.5},
            {u"initialThreshold"_s, 0.5},
        }));

        QList<int> rows(16);
        u::iota(rows, 0);
        QVERIFY(createPlot(rows));

        QCOMPARE(plottableNames<QCPBars>(), (QStringList{u"v00, v01 and 14 more…"_s, u"w"_s}));
        QCOMPARE(values<QCPBars>(u"v00, v01 and 14 more…"_s), QStringList{u"16"_s});
    }

    // The plot can be saved as an image, of the rows shown
    void saveImage_data()
    {
        QTest::addColumn<QString>("fileName");
        QTest::addColumn<QString>("extension");
        QTest::addColumn<QString>("savedFileName");
        QTest::addColumn<QByteArray>("header");

        QTest::newRow("PNG") << u"plot.png"_s << u"png"_s << u"plot.png"_s << QByteArray("\x89PNG");
        QTest::newRow("PDF") << u"plot.pdf"_s << u"pdf"_s << u"plot.pdf"_s << QByteArray("%PDF");
        QTest::newRow("JPEG") << u"plot.jpg"_s << u"jpg"_s << u"plot.jpg"_s << QByteArray("\xFF\xD8");

        // The extension is added, unless it's there already, in any case
        QTest::newRow("No extension") << u"plot"_s << u"png"_s << u"plot.png"_s << QByteArray("\x89PNG");
        QTest::newRow("Upper case") << u"plot.PNG"_s << u"png"_s << u"plot.png"_s << QByteArray("\x89PNG");
        QTest::newRow("Other extension") << u"plot.txt"_s << u"png"_s << u"plot.txt.png"_s << QByteArray("\x89PNG");
    }

    void saveImage()
    {
        QFETCH(const QString, fileName);
        QFETCH(const QString, extension);
        QFETCH(const QString, savedFileName);
        QFETCH(const QByteArray, header);

        QVERIFY(open());
        QVERIFY(createPlot({0, 2}));

        QVERIFY(execute([&]
        {
            QMetaObject::invokeMethod(_plot.get(), "savePlotImage",
                Q_ARG(QUrl, QUrl::fromLocalFile(_dir.filePath(fileName))), Q_ARG(QString, extension));
        }));

        QFile file(_dir.filePath(savedFileName));
        QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(savedFileName));
        QVERIFY(file.readAll().startsWith(header));

        // ...and then shown
        QCOMPARE(_urlCatcher._urls, QList<QUrl>{QUrl::fromLocalFile(_dir.filePath(savedFileName))});
    }

    // An image of each row, or each value of an attribute, named after it
    void saveImages_data()
    {
        QTest::addColumn<QString>("attributeName");
        QTest::addColumn<QStringList>("fileNames");

        QTest::newRow("By row") << QString{} << QStringList{u"images-r1.png"_s, u"images-r2.png"_s, u"images-r3.png"_s};
        QTest::newRow("By attribute") << u"Group"_s << QStringList{u"images-g1.png"_s, u"images-g2.png"_s};
    }

    void saveImages()
    {
        QFETCH(const QString, attributeName);
        QFETCH(const QStringList, fileNames);

        QVERIFY(open());
        QVERIFY(createPlot({0, 1, 2}));

        const auto url = QUrl::fromLocalFile(_dir.filePath(u"images.png"_s));
        QVERIFY(execute([&]
        {
            if(attributeName.isEmpty())
            {
                QMetaObject::invokeMethod(_plot.get(), "savePlotImageByRow",
                    Q_ARG(QUrl, url), Q_ARG(QString, u"png"_s));
            }
            else
            {
                QMetaObject::invokeMethod(_plot.get(), "savePlotImageByAttribute",
                    Q_ARG(QUrl, url), Q_ARG(QString, u"png"_s), Q_ARG(QString, attributeName));
            }
        }));

        for(const auto& fileName : fileNames)
            QVERIFY2(QFile::exists(_dir.filePath(fileName)), qPrintable(fileName));

        QVERIFY(!QFile::exists(url.toLocalFile()));

        // The folder they're in is shown
        QCOMPARE(_urlCatcher._urls, QList<QUrl>{QUrl::fromLocalFile(_dir.path())});

        // The plot itself is unchanged
        QCOMPARE(plottableNames<QCPGraph>(), (QStringList{u"r1"_s, u"r2"_s, u"r3"_s}));
    }
};

QTEST_MAIN(TestCorrelationPlot)
#include "tst_correlationplot.moc"
