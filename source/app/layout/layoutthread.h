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

#ifndef LAYOUTTHREAD_H
#define LAYOUTTHREAD_H

#include "shared/graph/elementid.h"
#include "shared/utils/performancecounter.h"

#include "nodepositions.h"
#include "layout.h"
#include "layoutfactory.h"
#include "layoutsettings.h"

#include <QObject>
#include <QString>
#include <QtGlobal>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

class ComponentSplitSet;
class Graph;
class GraphModel;

class LayoutThread : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool firstIterDone MEMBER _firstIterDone NOTIFY firstIterDone)
    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)

private:
    GraphModel* _graphModel = nullptr;
    mutable std::mutex _mutex;
    std::thread _thread;
    bool _pause = false;
    bool _paused = false;
    bool _stop = false;
    bool _repeating = false;
    std::condition_variable _waitForPause;
    std::condition_variable _waitForResume;

    std::unique_ptr<LayoutFactory> _layoutFactory;
    ComponentIdMap<std::unique_ptr<Layout>> _layouts;
    ComponentArray<bool> _executedAtLeastOnce;
    std::atomic_bool _firstIterDone = false;

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

    ~LayoutThread() override;

    void pause();
    void pauseAndWait();
    bool paused() const;
    void resume();

    void start();
    void stop();

    bool finished() const;

    void addAllComponents();

    void setNodePositions(const ExactNodePositions& nodePositions);

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
    void firstIterDone();
    void executed();
    void pausedChanged();
    void settingChanged(const QString& name, float value);
};

#endif // LAYOUTTHREAD_H
