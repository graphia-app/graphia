#include "pairwisesaver.h"

#include "shared/attributes/iattribute.h"
#include "shared/graph/igraph.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "ui/document.h"

#include <QFile>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>

static QString escape(QString string)
{
    string.replace(QStringLiteral("\""), QStringLiteral("\\\""));
    return string;
}

bool PairwiseSaver::save()
{
    QFile file(_url.toLocalFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);

    QTextStream stream(&file);
    int edgeCount = _graphModel->graph().numEdges();
    int runningCount = 0;

    _graphModel->mutableGraph().setPhase(QObject::tr("Edges"));
    for(auto edgeId : _graphModel->graph().edgeIds())
    {
        auto& edge = _graphModel->graph().edgeById(edgeId);
        auto sourceName = escape(_graphModel->nodeName(edge.sourceId()));
        auto targetName = escape(_graphModel->nodeName(edge.targetId()));

        if(sourceName.isEmpty())
            sourceName = QString::number(static_cast<int>(edge.sourceId()));
        if(targetName.isEmpty())
            targetName = QString::number(static_cast<int>(edge.targetId()));

        if(_graphModel->attributeExists(QStringLiteral("Edge Weight")) &&
           _graphModel->attributeByName(QStringLiteral("Edge Weight"))->valueType() == ValueType::Numerical)
        {
            auto* attribute = _graphModel->attributeByName(QStringLiteral("Edge Weight"));
            stream << QStringLiteral("\"%1\"").arg(sourceName) << " "
                   << QStringLiteral("\"%1\"").arg(targetName) << " " << attribute->floatValueOf(edgeId)
                   << endl;
        }
        else
            stream << QStringLiteral("\"%1\"").arg(sourceName) << " "
                   << QStringLiteral("\"%1\"").arg(targetName) << endl;

        runningCount++;
        setProgress(runningCount * 100 / edgeCount);
    }

    return true;
}

std::unique_ptr<ISaver> PairwiseSaverFactory::create(const QUrl& url, Document* document,
                                                    const IPluginInstance*, const QByteArray&,
                                                    const QByteArray&)
{
    return std::make_unique<PairwiseSaver>(url, document->graphModel());
}
