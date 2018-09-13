#include "pairwiseexporter.h"

#include "shared/attributes/iattribute.h"
#include "shared/graph/igraph.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"

#include <QFile>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>

static QString escape(QString string)
{
    string.replace(QStringLiteral("\""), QStringLiteral("\\\""));
    return string;
}

bool PairwiseExporter::save(const QUrl& url, IGraphModel* graphModel)
{
    QFile file(url.toLocalFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);

    QTextStream stream(&file);
    int edgeCount = graphModel->graph().numEdges();
    int runningCount = 0;

    graphModel->mutableGraph().setPhase(QObject::tr("Edges"));
    for(auto edgeId : graphModel->graph().edgeIds())
    {
        auto& edge = graphModel->graph().edgeById(edgeId);
        auto sourceName = escape(graphModel->nodeName(edge.sourceId()));
        auto targetName = escape(graphModel->nodeName(edge.targetId()));

        if(sourceName.isEmpty())
            sourceName = QString::number(static_cast<int>(edge.sourceId()));
        if(targetName.isEmpty())
            targetName = QString::number(static_cast<int>(edge.targetId()));

        if(graphModel->attributeExists(QStringLiteral("Edge Weight")) &&
           graphModel->attributeByName(QStringLiteral("Edge Weight"))->valueType() == ValueType::Numerical)
        {
            auto* attribute = graphModel->attributeByName(QStringLiteral("Edge Weight"));
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

QString PairwiseExporter::name() const { return QStringLiteral("Pairwise Text"); }

QString PairwiseExporter::extension() const { return QStringLiteral(".txt"); }
