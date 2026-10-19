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

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariant>

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

// A minimal plugin instance, which saves nothing, and records the data it's given to load
class TestPluginInstance : public IPluginInstance
{
private:
    const IPlugin* _plugin = nullptr;
    QByteArray _loadedData;
    int _loadedDataVersion = -1;

public:
    explicit TestPluginInstance(const IPlugin* plugin) : _plugin(plugin) {}

    void initialise(const IPlugin*, IDocument*, const IParserThread*) override {}
    std::unique_ptr<IParser> parserForUrlTypeName(const QString&) override { return nullptr; }

    void applyParameter(const QString&, const QVariant&) override {}

    QStringList defaultTransforms() const override { return {}; }
    QStringList defaultVisualisations() const override { return {}; }

    QByteArray save(IMutableGraph&, Progressable&) const override { return {}; }
    bool load(const QByteArray& data, int dataVersion, IMutableGraph&, IParser&) override
    {
        _loadedData = data;
        _loadedDataVersion = dataVersion;
        return true;
    }

    const IPlugin* plugin() override { return _plugin; }

    const QByteArray& loadedData() const { return _loadedData; }
    int loadedDataVersion() const { return _loadedDataVersion; }
};

#endif // TESTPLUGIN_H
