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

#ifndef LAYOUT_H
#define LAYOUT_H

#include "shared/graph/igraphcomponent.h"
#include "shared/graph/elementid_containers.h"
#include "graph/componentmanager.h"
#include "nodepositions.h"

#include "shared/utils/performancecounter.h"
#include "shared/utils/cancellable.h"
#include "shared/utils/enumbitmask.h"

#include "layoutsettings.h"

#include <QVector2D>
#include <QVector3D>
#include <QObject>

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <algorithm>
#include <limits>
#include <cstdint>
#include <set>
#include <map>

struct LayoutSettingKeyValue
{
    QString _name;
    float _value;
};

class Layout : public QObject, public Cancellable
{
    Q_OBJECT

public:
    enum class Iterative
    {
        Yes,
        No
    };

    enum class Dimensionality
    {
        ThreeDee        = 0x1,
        TwoDee          = 0x2,
        TwoOrThreeDee   = TwoDee | ThreeDee
    };

private:
    Iterative _iterative;
    Dimensionality _dimensionality;
    float _scaling;
    size_t _smoothing;
    const IGraphComponent* _graphComponent;
    NodeLayoutPositions* _positions;

protected:
    const LayoutSettings* _settings; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes

    NodeLayoutPositions& positions() { return *_positions; }

public:
    Layout(const IGraphComponent& graphComponent,
           NodeLayoutPositions& positions,
           const LayoutSettings* settings = nullptr,
           Iterative iterative = Iterative::No,
           Dimensionality dimensionality = Dimensionality::TwoOrThreeDee,
           float scaling = 1.0f,
           size_t smoothing = 1) :
        _iterative(iterative),
        _dimensionality(dimensionality),
        _scaling(scaling),
        _smoothing(smoothing),
        _graphComponent(&graphComponent),
        _positions(&positions),
        _settings(settings)
    {}

    float scaling() const { return _scaling; }
    size_t smoothing() const { return _smoothing; }

    const IGraphComponent& graphComponent() const { return *_graphComponent; }
    const std::vector<NodeId>& nodeIds() const { return _graphComponent->nodeIds(); }
    const std::vector<EdgeId>& edgeIds() const { return _graphComponent->edgeIds(); }

    virtual void execute(bool firstIteration, Dimensionality dimensionalityMode) = 0;

    // Indicates that the algorithm is doing no useful work
    virtual bool finished() const { return false; }

    // Resets the state of the algorithm such that finished() no longer returns true
    virtual void unfinish() { qFatal("unfinish not implemented"); }

    virtual bool iterative() const { return _iterative == Iterative::Yes; }
    virtual Dimensionality dimensionality() const { return _dimensionality; }

signals:
    void progress(int percentage);
};

class GraphModel;

class LayoutFactory
{
protected:
    GraphModel* _graphModel = nullptr; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes
    LayoutSettings _layoutSettings; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes

public:
    explicit LayoutFactory(GraphModel* graphModel) :
        _graphModel(graphModel)
    {}

    virtual ~LayoutFactory() = default;

    LayoutSettings& settings()
    {
        return _layoutSettings;
    }

    const LayoutSetting* setting(const QString& name) const
    {
        return _layoutSettings.setting(name);
    }

    void setSettingValue(const QString& name, float value)
    {
        _layoutSettings.setValue(name, value);
    }

    void setSettingNormalisedValue(const QString& name, float normalisedValue)
    {
        _layoutSettings.setNormalisedValue(name, normalisedValue);
    }

    void resetSettingValue(const QString& name)
    {
        _layoutSettings.resetValue(name);
    }

    virtual QString name() const = 0;
    virtual QString displayName() const = 0;
    virtual std::unique_ptr<Layout> create(ComponentId componentId,
        NodeLayoutPositions& results, Layout::Dimensionality dimensionalityMode) = 0;
};

class LayoutThread : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)

private:
    GraphModel* _graphModel = nullptr;
    mutable std::mutex _mutex;
    std::thread _thread;
    bool _started = false;
    bool _pause = false;
    bool _paused = true;
    bool _stop = false;
    bool _repeating = false;
    std::condition_variable _waitForPause;
    std::condition_variable _waitForResume;

    std::unique_ptr<LayoutFactory> _layoutFactory;
    ComponentIdMap<std::unique_ptr<Layout>> _layouts;
    ComponentArray<bool> _executedAtLeastOnce;

    Layout::Dimensionality _dimensionalityMode =
        Layout::Dimensionality::ThreeDee;

    NodeLayoutPositions _nodeLayoutPositions;

    PerformanceCounter _performanceCounter;

    bool _layoutPotentiallyRequired = false;

    int _debug = 0;

public:
    LayoutThread(GraphModel& graphModel,
                 std::unique_ptr<LayoutFactory>&& layoutFactory,
                 bool repeating = false);

    ~LayoutThread() override
    {
        stop();

        if(_thread.joinable())
            _thread.join();
    }

    void pause();
    void pauseAndWait();
    bool paused() const;
    void resume();

    void start();
    void stop();

    bool finished() const;

    void addAllComponents();

    void setStartingNodePositions(const ExactNodePositions& nodePositions);

    Layout::Dimensionality dimensionalityMode();
    void setDimensionalityMode(Layout::Dimensionality dimensionalityMode);

    QString layoutName() const;
    QString layoutDisplayName() const;

    std::vector<LayoutSetting>& settings();
    const LayoutSetting* setting(const QString& name) const;

    void setSettingValue(const QString& name, float value);
    void setSettingNormalisedValue(const QString& name, float normalisedValue);
    void resetSettingValue(const QString& name);

private:
    bool iterative() const;
    bool allLayoutsFinished() const;
    bool workToDo() const;
    void uncancel();
    void unfinish();
    void run();

    void addComponent(ComponentId componentId);
    void removeComponent(ComponentId componentId);

private slots:
    void onComponentSplit(const Graph*, const ComponentSplitSet& componentSplitSet);
    void onComponentAdded(const Graph*, ComponentId componentId, bool);
    void onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool);

signals:
    void executed();
    void pausedChanged();
    void settingChanged(const QString& name, float value);
};

#endif // LAYOUT_H
