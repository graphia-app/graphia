#ifndef FILETYPEIDENTIFIER_H
#define FILETYPEIDENTIFIER_H

#include <QString>
#include <QFileInfo>

#include <vector>
#include <memory>
#include <tuple>

class FileIdentifier
{
public:
    class Type
    {
    public:
        Type(const QString& name,
             const QString& collectiveDescription,
             const std::vector<QString>& extensions) :
            _name(name),
            _collectiveDescription(collectiveDescription),
            _extensions(extensions)
        {}

        const QString& name() const { return _name; }
        const QString& collectiveDescription() const { return _collectiveDescription; }
        const std::vector<QString> extensions() const { return _extensions; }

        virtual bool identify(const QFileInfo& fileInfo) const = 0;

    private:
        QString _name;
        QString _collectiveDescription;
        std::vector<QString> _extensions;
    };

    FileIdentifier();

    void registerFileType(const std::shared_ptr<Type> fileType);
    const std::tuple<Type*, QString> identify(const QString& filename) const;
    const QString& filter() const { return _filter; }

private:
    std::vector<std::shared_ptr<Type>> _fileTypes;
    QString _filter;
};

#endif // FILETYPEIDENTIFIER_H
