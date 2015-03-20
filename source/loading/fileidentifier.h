#ifndef FILETYPEIDENTIFIER_H
#define FILETYPEIDENTIFIER_H

#include <QString>

#include <memory>

class FileIdentifier
{
public:
    class Type
    {
    public:
        Type(const QString& name) :
            _name(name)
        {}

        const QString& name() const { return _name; }
        virtual bool identify(const QString& filename) = 0;

    private:
        QString _name;
    };

public:
    FileIdentifier();

    void registerFileType(const std::shared_ptr<Type> fileType);
    QString identify(const QString& filename);

private:
    std::vector<std::shared_ptr<Type>> _fileTypes;
};

#endif // FILETYPEIDENTIFIER_H
