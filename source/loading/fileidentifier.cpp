#include "fileidentifier.h"

#include <QFileInfo>

FileIdentifier::FileIdentifier()
{
}

void FileIdentifier::registerFileType(const std::shared_ptr<Type> fileType)
{
    _fileTypes.push_back(fileType);

    // Sort by collective description
    std::sort(_fileTypes.begin(), _fileTypes.end(),
    [](const std::shared_ptr<Type>& a, const std::shared_ptr<Type>& b)
    {
        return a->collectiveDescription().compare(b->collectiveDescription(), Qt::CaseInsensitive);
    });

    _filter.clear();
    _filter += QObject::tr("All Files (");
    bool second = false;

    for(auto fileType : _fileTypes)
    {
        for(auto extension : fileType->extensions())
        {
            if(second)
                _filter += " ";
            else
                second = true;

            _filter += "*." + extension;
        }
    }

    _filter += ")";

    for(auto fileType : _fileTypes)
    {
        _filter += ";;" + fileType->collectiveDescription() + " (";

        for(auto extension : fileType->extensions())
            _filter += "*." + extension;

        _filter += ")";
    }
}

const std::tuple<FileIdentifier::Type*, QString> FileIdentifier::identify(const QString& filename) const
{
    QFileInfo info(filename);

    if(info.exists())
    {
        for(auto fileType : _fileTypes)
        {
            if(fileType->identify(filename))
                return std::make_tuple(fileType.get(), info.fileName());
        }
    }

    return std::make_tuple( nullptr, info.fileName());
}
