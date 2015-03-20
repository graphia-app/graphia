#include "fileidentifier.h"

FileIdentifier::FileIdentifier()
{
}

void FileIdentifier::registerFileType(const std::shared_ptr<Type> fileType)
{
    _fileTypes.push_back(fileType);
}

QString FileIdentifier::identify(const QString& filename)
{
    for(auto fileType : _fileTypes)
    {
        if(fileType->identify(filename))
            return fileType->name();
    }

    return "";
}
