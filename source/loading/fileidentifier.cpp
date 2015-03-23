#include "fileidentifier.h"

#include <QFileInfo>

FileIdentifier::FileIdentifier()
{
}

void FileIdentifier::registerFileType(const std::shared_ptr<Type> fileType)
{
    _fileTypes.push_back(fileType);
}

const FileIdentifier::Type* FileIdentifier::identify(const QString& filename)
{
    QFileInfo info(filename);

    if(info.exists())
    {
        for(auto fileType : _fileTypes)
        {
            if(fileType->identify(filename))
                return fileType.get();
        }
    }

    return nullptr;
}
