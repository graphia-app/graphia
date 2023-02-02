/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef PROGRESSABLE_H
#define PROGRESSABLE_H

#include <QString>

#include <functional>

using ProgressFn = std::function<void(int)>;
using PhaseFn = std::function<void(const QString&)>;

class Progressable
{
private:
    ProgressFn _progressFn = [](int) {};
    PhaseFn _phaseFn = [](const QString&) {};

public:
    virtual ~Progressable() = default;

    virtual void setProgress(int percent) { _progressFn(percent); }
    void setProgressFn(const ProgressFn& f) { _progressFn = f; }

    // Informational messages to indicate progress
    virtual void setPhase(const QString& phase) { _phaseFn(phase); }
    void clearPhase() { setPhase({}); }
    void setPhaseFn(const PhaseFn& f) { _phaseFn = f; }
};

#endif // PROGRESSABLE_H
