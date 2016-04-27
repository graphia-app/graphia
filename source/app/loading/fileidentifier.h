#ifndef FILETYPEIDENTIFIER_H
#define FILETYPEIDENTIFIER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QFileInfo>

#include <vector>
#include <memory>

class FileIdentifier : public QObject
{
    Q_OBJECT
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

        virtual ~Type() {}

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

    void registerFileType(const std::shared_ptr<Type>& newFileType);
    std::vector<const Type*> identify(const QString& filename) const;
    const QStringList nameFilters() const { return _nameFilters; }
    const QStringList fileTypeNames() const;

signals:
    void nameFiltersChanged();

private:
    std::vector<std::shared_ptr<Type>> _fileTypes;
    QStringList _nameFilters;
};

#endif // FILETYPEIDENTIFIER_H
