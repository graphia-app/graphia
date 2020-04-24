#include "saverfactory.h"

#include "ui/document.h"
#include "shared/graph/igraphmodel.h"

IGraphModel* graphModelFor(Document* document)
{
    return document->graphModel();
}
