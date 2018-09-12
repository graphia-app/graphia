#include "pairwiseexporter.h"

#include "shared/graph/igraph.h"
#include "shared/graph/igraphmodel.h"
#include "shared/attributes/iattribute.h"

#include <QFile>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>

static QString escape(const QString& unescaped)
{
    QString escaped = unescaped;
    escaped.replace("\"", "\\\"");
    return escaped;
}

bool PairwiseExporter::save(const QUrl &url, IGraphModel *graphModel)
{
    QFile file(url.toLocalFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);

    QTextStream stream(&file);
    size_t edgeCount = static_cast<size_t>(graphModel->graph().numEdges());
    size_t runningCount = 0;

    for(auto edgeId : graphModel->graph().edgeIds())
    {
        auto& edge = graphModel->graph().edgeById(edgeId);
        auto sourceName = escape(graphModel->nodeName(edge.sourceId()));
        auto targetName = escape(graphModel->nodeName(edge.targetId()));

        if(sourceName.isEmpty())
            sourceName = QString::number(static_cast<int>(edge.sourceId()));
        if(targetName.isEmpty())
            targetName = QString::number(static_cast<int>(edge.targetId()));

        if(graphModel->attributeExists("Edge Weight") && graphModel->attributeByName("Edge Weight")->valueType() == ValueType::Numerical)
        {
            auto* attribute = graphModel->attributeByName("Edge Weight");
            stream << QString("\"%1\"").arg(sourceName) << " " << QString("\"%1\"").arg(targetName) << " " << attribute->floatValueOf(edgeId) << endl;
        }
        else
            stream << QString("\"%1\"").arg(sourceName) << " " << QString("\"%1\"").arg(targetName) << endl;

        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / edgeCount));
    }

    return true;
}

QString PairwiseExporter::name() const
{
    return "Pairwise Text";
}

QString PairwiseExporter::extension() const
{
    return ".txt";
}
