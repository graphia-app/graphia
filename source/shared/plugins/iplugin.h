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

#ifndef IPLUGIN_H
#define IPLUGIN_H

#include "build_defines.h"

#include "shared/iapplication.h"

#include "shared/loading/iurltypes.h"
#include "shared/loading/iparser.h"

#include "shared/utils/failurereason.h"

#include <QtPlugin>
#include <QString>
#include <QStringList>
#include <QByteArray>

#include <memory>

class IPlugin;
class IDocument;
class IParserThread;
class IMutableGraph;
class QObject;
class QUrl;

class IPluginInstance : public FailureReason
{
public:
    ~IPluginInstance() override = default;

    virtual void initialise(const IPlugin* plugin, IDocument* document,
                            const IParserThread* parserThread) = 0;
    virtual std::unique_ptr<IParser> parserForUrlTypeName(const QString& urlTypeName) = 0;

    virtual void applyParameter(const QString& name, const QVariant& value) = 0;

    virtual QStringList defaultTransforms() const = 0;
    virtual QStringList defaultVisualisations() const = 0;

    virtual QByteArray save(IMutableGraph& mutableGraph, Progressable& progressable) const = 0;
    virtual bool load(const QByteArray& data, int dataVersion,
        IMutableGraph& mutableGraph, IParser& parser) = 0;

    virtual const IPlugin* plugin() = 0;
};

class IPluginInstanceProvider
{
public:
    virtual ~IPluginInstanceProvider() = default;

    virtual std::unique_ptr<IPluginInstance> createInstance() = 0;
};

class IPlugin : public virtual IUrlTypes, public virtual IPluginInstanceProvider
{
public:
    ~IPlugin() override = default;

    virtual void initialise(const IApplication* application) = 0;

    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QString imageSource() const = 0; // Displayed in the about dialog

    virtual int dataVersion() const = 0;

    virtual QStringList identifyUrl(const QUrl& url) const = 0;
    virtual QString failureReason(const QUrl& url) const = 0;

    virtual bool editable() const = 0;
    virtual bool directed() const = 0;

    virtual QString parametersQmlPath(const QString& urlType) const = 0;
    virtual QString qmlPath() const = 0;

    virtual const IApplication* application() const = 0;

    virtual QObject* ptr() = 0;
};

#define IPluginIID(NAME) "app.graphia.IPlugin:" #NAME "/" VERSION // NOLINT cppcoreguidelines-macro-usage
Q_DECLARE_INTERFACE(IPlugin, IPluginIID(Base)) // NOLINT cppcoreguidelines-pro-type-const-cast

#endif // IPLUGIN_H
