#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

#include "findoptions.h"

#include "shared/graph/elementid.h"
#include "shared/graph/elementid_containers.h"
#include "shared/utils/flags.h"

#include <QObject>
#include <QString>
#include <QStringList>

class GraphModel;

class SearchManager : public QObject
{
    Q_OBJECT
public:
    explicit SearchManager(const GraphModel& graphModel);

    void findNodes(const QString& term, Flags<FindOptions> options,
        QStringList attributeNames);
    void clearFoundNodeIds();
    void refresh();

    const NodeIdSet& foundNodeIds() const { return _foundNodeIds; }
    bool nodeWasFound(NodeId nodeId) const;

    bool active() const { return !_term.isEmpty(); }

private:
    QString _term;
    Flags<FindOptions> _options;
    QStringList _attributeNames;

    const GraphModel* _graphModel = nullptr;
    NodeIdSet _foundNodeIds;

signals:
    void foundNodeIdsChanged(const SearchManager*);
};

#endif // SEARCHMANAGER_H
