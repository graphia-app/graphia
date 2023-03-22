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

#include "headless.h"

#include "application.h"
#include "app/ui/document.h"

#include "shared/utils/container.h"

#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QFileInfo>
#include <QDir>

#include <json_helper.h>

struct HeadlessState
{
    Application _application;
    Document* _document = nullptr;

    bool _saveComplete = false;

    QStringList _sourceFilenames;
    QString _parametersFilename;

    QString _type;
    QString _pluginName;
    QVariantMap _pluginParameters;
    QString _destination;

    HeadlessState(const QStringList& sourceFilenames, const QString& parametersFilename) :
        _sourceFilenames(sourceFilenames), _parametersFilename(parametersFilename)
    {}

    void reset()
    {
        _document = new Document;
        _document->setProperty("application", QVariant::fromValue(&_application));

        _saveComplete = false;
    }
};

Headless::Headless(const QStringList& sourceFilenames, const QString& parametersFilename) :
    _(std::make_unique<HeadlessState>(sourceFilenames, parametersFilename))
{}

Headless::~Headless() // NOLINT
{
    // Only here so that we can have a unique_ptr to HeadlessImpl
}

void Headless::processNext()
{
    _->reset();

    connect(_->_document, &Document::loadComplete, [this]
    {
        const auto& source = _->_sourceFilenames.at(0);
        QFileInfo sfi(source);
        QFileInfo dfi(_->_destination);
        QString target;

        if(dfi.exists() && dfi.isDir())
        {
            target = QStringLiteral("%1/%2.%3").arg(
                _->_destination, sfi.baseName(), Application::nativeExtension());
        }
        else if(_->_destination.isEmpty())
        {
            target = QStringLiteral("%1/%2.%3").arg(
                sfi.dir().path(), sfi.baseName(), Application::nativeExtension());
        }
        else
            target = _->_destination;

        if(!QFileInfo(target).isWritable())
        {
            std::cerr << target.toStdString() << " is not writable.\n";
            emit done();
            return;
        }

        _->_document->saveFile(QUrl::fromLocalFile(target), Application::name(), {}, {});
    });

    connect(_->_document, &Document::saveComplete, this, [this]
    {
        _->_document->deleteLater();
        _->_document = nullptr;

        _->_sourceFilenames.pop_front();
        if(_->_sourceFilenames.isEmpty())
            emit done();
        else
            processNext();
    });

    bool success = _->_document->openUrl(QUrl::fromLocalFile(_->_sourceFilenames.at(0)),
        _->_type, _->_pluginName, _->_pluginParameters);

    if(!success)
    {
        std::cerr << "Failed to open " << _->_sourceFilenames.at(0).toStdString() << ".\n";
        emit done();
    }
}

void Headless::run()
{
    if(_->_sourceFilenames.isEmpty())
    {
        emit done();
        return;
    }

    auto parameters = parseJsonFrom(_->_parametersFilename);
    if(parameters.is_null())
    {
        emit done();
        return;
    }

    if(!u::containsAllOf(parameters, {"type", "plugin"}))
    {
        std::cerr << "Parameters file must specify 'type' and 'plugin'.\n";
        emit done();
        return;
    }

    auto plugin = parameters["plugin"];

    if(!u::contains(plugin, "name"))
    {
        std::cerr << "Parameters file 'plugin' must specify 'name'.\n";
        emit done();
        return;
    }

    _->_type = QString::fromStdString(parameters["type"]);
    _->_pluginName = QString::fromStdString(plugin["name"]);
    _->_destination = u::contains(parameters, "destination") ?
        QString::fromStdString(parameters["destination"]) : QString();

    if(u::contains(plugin, "parameters"))
    {
        if(auto j = plugin["parameters"]; j.is_object())
        {
            for(const auto& e : j.items())
            {
                auto key = QString::fromStdString(e.key());
                QVariant value;
                from_json(e.value(), value);
                _->_pluginParameters.insert(key, value);
            }
        }
    }

    QFileInfo dfi(_->_destination);
    if(_->_sourceFilenames.size() > 1 && (!dfi.exists() || !dfi.isDir() || !dfi.isWritable()))
    {
        std::cerr << "When processing multiple files, 'destination' must be an existing writable directory.\n";
        emit done();
        return;
    }

    processNext();
}
