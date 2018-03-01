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

// What to select when found nodes changes
DEFINE_QML_ENUM(Q_GADGET, FindSelectStyle,
                None, First, All);

class SearchManager : public QObject
{
    Q_OBJECT
public:
    explicit SearchManager(const GraphModel& graphModel);

    void findNodes(const QString& term, Flags<FindOptions> options,
        QStringList attributeNames, FindSelectStyle selectStyle);
    void clearFoundNodeIds();
    void refresh();

    const NodeIdSet& foundNodeIds() const { return _foundNodeIds; }
    bool nodeWasFound(NodeId nodeId) const;

    bool active() const { return !_term.isEmpty(); }

    FindSelectStyle selectStyle() const { return _selectStyle; }

private:
    QString _term;
    Flags<FindOptions> _options;
    QStringList _attributeNames;
    FindSelectStyle _selectStyle;

    const GraphModel* _graphModel = nullptr;
    NodeIdSet _foundNodeIds;

signals:
    void foundNodeIdsChanged(const SearchManager*);
};

#endif // SEARCHMANAGER_H
