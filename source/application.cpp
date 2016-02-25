#include "application.h"

#include "loading/gmlfiletype.h"
#include "loading/pairwisetxtfiletype.h"

#include "loading/gmlfileparser.h"
#include "loading/pairwisetxtfileparser.h"

#include "graph/graphmodel.h"
#include "graph/weightededgegraphmodel.h"

#include "utils/cpp1x_hacks.h"
#include "utils/preferences.h"

#include <memory>
#include "loading/gmlfileparser.h"

#include <cmath>

const char* Application::_uri = "com.kajeka";

Application::Application(QObject *parent) :
    QObject(parent)
{
    connect(&_fileIdentifier, &FileIdentifier::nameFiltersChanged, this, &Application::nameFiltersChanged);
    connect(Preferences::instance(), &Preferences::preferenceChanged, this, &Application::textColorChanged);

    _fileIdentifier.registerFileType(std::make_shared<GmlFileType>());
    _fileIdentifier.registerFileType(std::make_shared<PairwiseTxtFileType>());
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

QColor Application::textColor() const
{
    auto diff = [](const QColor& a, const QColor& b)
    {
        auto ay = 0.299f * a.redF() + 0.587f * a.greenF() + 0.114f * a.blueF();
        auto by = 0.299f * b.redF() + 0.587f * b.greenF() + 0.114f * b.blueF();

        return std::abs(ay - by);
    };

    auto background = u::pref("visualDefaults/backgroundColor").value<QColor>();
    auto blackDiff = diff(background, Qt::GlobalColor::black);
    auto whiteDiff = diff(background, Qt::GlobalColor::white);

    if(blackDiff > whiteDiff)
        return Qt::GlobalColor::black;

    return Qt::GlobalColor::white;
}
