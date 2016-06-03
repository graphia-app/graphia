#include "application.h"

#include "loading/gmlfiletype.h"
#include "loading/pairwisetxtfiletype.h"

#include "loading/gmlfileparser.h"
#include "loading/pairwisetxtfileparser.h"

#include "graph/graphmodel.h"
#include "graph/weightededgegraphmodel.h"

#include "utils/preferences.h"

#include "loading/gmlfileparser.h"

#include "../plugins/iplugin.h"

#include <QPluginLoader>
#include <QDir>
#include <QStandardPaths>

#include <memory>
#include <cmath>

const char* Application::_uri = "com.kajeka";

Application::Application(QObject *parent) :
    QObject(parent)
{
    _localPluginsDir = QStandardPaths::writableLocation(
                QStandardPaths::StandardLocation::AppDataLocation) + "/plugins";
    QDir().mkpath(_localPluginsDir);

    connect(&_fileIdentifier, &FileIdentifier::nameFiltersChanged, this, &Application::nameFiltersChanged);

    _fileIdentifier.registerFileType(std::make_shared<GmlFileType>());
    _fileIdentifier.registerFileType(std::make_shared<PairwiseTxtFileType>());

    loadPlugins();
}

bool Application::parserAndModelForFile(const QUrl& url, const QString& fileTypeName,
                                        std::unique_ptr<GraphFileParser>& graphFileParser,
                                        std::shared_ptr<GraphModel>& graphModel) const
{
    //FIXME what we should be doing:
    // query which plugins can load fileTypeName
    // allow the user to choose which plugin to use if there is more than 1
    // then something like:
    // graphModel = plugin->graphModelForFilename(filename);
    // graphFileParser = plugin->parserForFilename(filename);

    QString fileName = url.toLocalFile();
    QString baseFileName = baseFileNameForUrl(url);

    if(fileTypeName.compare("GML") == 0)
    {
        graphFileParser = std::make_unique<GmlFileParser>(fileName);
        graphModel = std::make_shared<GraphModel>(baseFileName);

        return true;
    }
    else if(fileTypeName.compare("PairwiseTXT") == 0)
    {
        auto weightedEdgeGraphModel = std::make_shared<WeightedEdgeGraphModel>(baseFileName);
        graphFileParser = std::make_unique<PairwiseTxtFileParser>(fileName, weightedEdgeGraphModel);
        graphModel = weightedEdgeGraphModel;

        return true;
    }

    return false;
}

bool Application::canOpen(const QString& fileTypeName) const
{
    //FIXME This is temporary (...probably)
    return _fileIdentifier.fileTypeNames().contains(fileTypeName);
}

bool Application::canOpenAnyOf(const QStringList& fileTypeNames) const
{
    for(auto fileTypeName : fileTypeNames)
    {
        if(canOpen(fileTypeName))
            return true;
    }

    return false;
}

QStringList Application::fileTypesOf(const QUrl& url) const
{
    QStringList fileTypes;

    for(auto fileType : _fileIdentifier.identify(url.toLocalFile()))
        fileTypes.append(fileType->name());

    return fileTypes;
}

void Application::loadPlugins()
{
    std::vector<QString> pluginsDirs =
    {
        qApp->applicationDirPath() + "/plugins",
        _localPluginsDir
    };

    for(auto& pluginsDir : pluginsDirs)
    {
        if(pluginsDir.isEmpty())
            continue;

        QDir pluginsQDir(pluginsDir);

        for(auto& fileName : pluginsQDir.entryList(QDir::Files))
        {
            QPluginLoader pluginLoader(pluginsQDir.absoluteFilePath(fileName));
            QObject* plugin = pluginLoader.instance();
            if(!pluginLoader.isLoaded())
                qDebug() << pluginLoader.errorString();

            if(plugin)
            {
                auto* iplugin = qobject_cast<IPlugin*>(plugin);
                if(iplugin)
                    _plugins.push_back(iplugin);
            }
        }
    }
}
