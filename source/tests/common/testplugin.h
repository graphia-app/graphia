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

#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

#include "shared/plugins/baseplugin.h"

#include <QString>
#include <QStringList>
#include <QUrl>

#include <memory>

// A minimal plugin, for constructing a GraphModel in tests
class TestPlugin : public BasePlugin // clazy:exclude=missing-qobject-macro
{
public:
    QString name() const override { return QStringLiteral("Test"); }
    QString displayName() const override { return QStringLiteral("Test"); }
    QString description() const override { return {}; }
    int dataVersion() const override { return 1; }

    QStringList identifyUrl(const QUrl&) const override { return {}; }
    QString failureReason(const QUrl&) const override { return {}; }

    bool editable() const override { return true; }

    std::unique_ptr<IPluginInstance> createInstance() override { return nullptr; }
};

#endif // TESTPLUGIN_H
