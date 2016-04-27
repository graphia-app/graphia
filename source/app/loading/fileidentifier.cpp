#include "fileidentifier.h"

#include <QFileInfo>

FileIdentifier::FileIdentifier()
{
}

void FileIdentifier::registerFileType(const std::shared_ptr<Type>& newFileType)
{
    _fileTypes.push_back(newFileType);

    // Sort by collective description
    std::sort(_fileTypes.begin(), _fileTypes.end(),
    [](const std::shared_ptr<Type>& a, const std::shared_ptr<Type>& b)
    {
        return a->collectiveDescription().compare(b->collectiveDescription(), Qt::CaseInsensitive) < 0;
    });

    QString description = QObject::tr("All Files (");
    bool second = false;

    for(auto fileType : _fileTypes)
    {
        for(auto extension : fileType->extensions())
        {
            if(second)
                description += " ";
            else
                second = true;

            description += "*." + extension;
        }
    }

    description += ")";

    _nameFilters.clear();
    _nameFilters.append(description);

    for(auto fileType : _fileTypes)
    {
        description = fileType->collectiveDescription() + " (";

        for(auto extension : fileType->extensions())
            description += "*." + extension;

        description += ")";

        _nameFilters.append(description);
    }

    emit nameFiltersChanged();
}

std::vector<const FileIdentifier::Type*> FileIdentifier::identify(const QString& filename) const
{
    QFileInfo info(filename);
    std::vector<const FileIdentifier::Type*> types;

    if(info.exists())
    {
        for(auto fileType : _fileTypes)
        {
            if(fileType->identify(filename))
                types.push_back(fileType.get());
        }
    }

    return types;
}

const QStringList FileIdentifier::fileTypeNames() const
{
    QStringList fileTypeNames;

    for(auto fileType : _fileTypes)
        fileTypeNames.append(fileType->name());

    return fileTypeNames;
}
