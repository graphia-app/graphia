#include "urltypes.h"

#include "shared/utils/container.h"

#include <QFileInfo>
#include <QUrl>

void UrlTypes::registerUrlType(const QString& urlTypeName,
                               const QString& individualDescription,
                               const QString& collectiveDescription,
                               const QStringList& extensions)
{
    _urlTypes.emplace(urlTypeName, UrlType(individualDescription, collectiveDescription, extensions));
}

QStringList UrlTypes::identifyByExtension(const QUrl& url) const
{
    QStringList urlTypeNames;

    if(url.isLocalFile())
    {
        QString extension = QFileInfo(url.toLocalFile()).suffix();

        for(const auto& urlType : _urlTypes)
        {
            for(auto& loadableExtension : urlType.second.extensions())
            {
                if(loadableExtension.compare(extension, Qt::CaseInsensitive) == 0)
                    urlTypeNames.append(urlType.first);
            }
        }
    }

    return urlTypeNames;
}

QStringList UrlTypes::loadableUrlTypeNames() const
{
    QStringList urlTypeNames;
    urlTypeNames.reserve(static_cast<int>(_urlTypes.size()));
    for(const auto& urlType : _urlTypes)
        urlTypeNames.append(urlType.first);

    return urlTypeNames;
}

QString UrlTypes::individualDescriptionForUrlTypeName(const QString& urlTypeName) const
{
    Q_ASSERT(u::contains(_urlTypes, urlTypeName));

    return _urlTypes.at(urlTypeName).individualDescription();
}

QString UrlTypes::collectiveDescriptionForUrlTypeName(const QString& urlTypeName) const
{
    Q_ASSERT(u::contains(_urlTypes, urlTypeName));

    return _urlTypes.at(urlTypeName).collectiveDescription();
}

QStringList UrlTypes::extensionsForUrlTypeName(const QString& urlTypeName) const
{
    Q_ASSERT(u::contains(_urlTypes, urlTypeName));

    return _urlTypes.at(urlTypeName).extensions();
}
