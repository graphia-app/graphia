/* Copyright © 2013-2020 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BIOPAXFILEPARSER_H
#define BIOPAXFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/plugins/basegenericplugin.h"
#include "shared/graph/imutablegraph.h"
#include "shared/plugins/userelementdata.h"

#include <QtXml/QXmlDefaultHandler>

#include <stack>

class BiopaxFileParser;

class BiopaxHandler : public QXmlDefaultHandler
{
private:
    struct TemporaryEdge
    {
        QStringList _sources;
        QStringList _targets;
        friend bool operator<(const TemporaryEdge& l, const TemporaryEdge& r)
        {
            return std::tie(l._sources, l._targets) < std::tie(r._sources, r._targets);
        }
    };

    BiopaxFileParser* _parser = nullptr;
    IGraphModel* _graphModel = nullptr;

    QString _errorString = QString();

    // Key = AttributeKeyID
    std::vector<TemporaryEdge> _temporaryEdges;
    std::map<QString, NodeId> _nodeMap;
    std::map<NodeId, QString> _nodeIdToNameMap;
    std::map<TemporaryEdge, EdgeId> _edgeIdMap;

    std::stack<NodeId> _activeNodes;
    std::stack<TemporaryEdge*> _activeTemporaryEdges;
    std::stack<QString> _activeElements;

    QXmlLocator* _locator = nullptr;
    int _lineCount = 0;

    UserNodeData* _userNodeData;
public:
    BiopaxHandler(BiopaxFileParser& parser, IGraphModel& graphModel, UserNodeData* userNodeData,
                  int lineCount);
    bool startDocument() override;
    bool endDocument() override;
    bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName,
                      const QXmlAttributes& atts) override;
    bool endElement(const QString& namespaceURI, const QString& localName, const QString& qName) override;
    bool characters(const QString& ch) override;
    void setDocumentLocator(QXmlLocator* locator) override;

    QString errorString() const override;
    bool warning(const QXmlParseException& exception) override;
    bool error(const QXmlParseException& exception) override;
    bool fatalError(const QXmlParseException& exception) override;
};

class BiopaxFileParser : public IParser
{

private:
    UserNodeData* _userNodeData;

public:
    explicit BiopaxFileParser(UserNodeData* userNodeData);

    bool parse(const QUrl& url, IGraphModel* graphModel) override;
    static bool canLoad(const QUrl&) { return true; }
};

#endif // BIOPAXFILEPARSER_H
