#ifndef IDOCUMENT_H
#define IDOCUMENT_H

#include "shared/graph/elementid.h"
#include "shared/utils/flags.h"

class IGraphModel;
class ISelectionManager;
class ICommandManager;
class QString;

// These values shadow the QMessageBox ones
enum class MessageBoxIcon
{
    NoIcon      = 0,
    Question    = 4,
    Information = 1,
    Warning     = 2,
    Critical    = 3
};

enum class MessageBoxButton
{
    None    = 0,
    Ok      = 0x00000400,
    Cancel  = 0x00400000,
    Close   = 0x00200000,
    Yes     = 0x00004000,
    No      = 0x00010000,
    Abort   = 0x00040000,
    Retry   = 0x00080000,
    Ignore  = 0x00100000
};

class IDocument
{
public:
    virtual ~IDocument() = default;

    virtual const IGraphModel* graphModel() const = 0;
    virtual IGraphModel* graphModel() = 0;

    virtual const ISelectionManager* selectionManager() const = 0;
    virtual ISelectionManager* selectionManager() = 0;

    virtual const ICommandManager* commandManager() const = 0;
    virtual ICommandManager* commandManager() = 0;

    virtual MessageBoxButton messageBox(MessageBoxIcon icon, const QString& title, const QString& text,
        Flags<MessageBoxButton> buttons = MessageBoxButton::Ok) = 0;

    virtual void moveFocusToNode(NodeId nodeId) = 0;
};

#endif // IDOCUMENT_H
