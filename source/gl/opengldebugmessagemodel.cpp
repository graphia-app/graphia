#include "opengldebugmessagemodel.h"

OpenGLDebugMessageModel::OpenGLDebugMessageModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int OpenGLDebugMessageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_messages.length();
}

int OpenGLDebugMessageModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 5;
}

QVariant OpenGLDebugMessageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const QOpenGLDebugMessage &message = m_messages.at(index.row());

    if (role != Qt::DisplayRole)
        return QVariant();

    switch (index.column()) {
    case 0:
        switch (message.severity()) {
        case QOpenGLDebugMessage::HighSeverity:
            return tr("High");
            break;
        case QOpenGLDebugMessage::NotificationSeverity:
            return tr("Notification");
            break;
        case QOpenGLDebugMessage::LowSeverity:
            return tr("Low");
            break;
        case QOpenGLDebugMessage::MediumSeverity:
            return tr("Medium");
            break;
        default:
            return tr("Invalid");
            break;
        }
        return QVariant();
        break;
    case 1:
        return message.id();
        break;
    case 2:
        switch (message.source()) {
        case QOpenGLDebugMessage::APISource:
            return tr("API");
            break;
        case QOpenGLDebugMessage::WindowSystemSource:
            return tr("Window system");
            break;
        case QOpenGLDebugMessage::ShaderCompilerSource:
            return tr("Shader compiler");
            break;
        case QOpenGLDebugMessage::ThirdPartySource:
            return tr("Third party");
            break;
        case QOpenGLDebugMessage::ApplicationSource:
            return tr("Application");
            break;
        case QOpenGLDebugMessage::OtherSource:
            return tr("Other");
            break;
        default:
            return tr("Invalid");
            break;
        }
        return QVariant();
        break;
    case 3:
        switch (message.type()) {
        case QOpenGLDebugMessage::ErrorType:
            return tr("Error");
            break;
        case QOpenGLDebugMessage::DeprecatedBehaviorType:
            return tr("Deprecated behavior");
            break;
        case QOpenGLDebugMessage::UndefinedBehaviorType:
            return tr("Undefined behavior");
            break;
        case QOpenGLDebugMessage::PortabilityType:
            return tr("Portability");
            break;
        case QOpenGLDebugMessage::PerformanceType:
            return tr("Performance");
            break;
        case QOpenGLDebugMessage::OtherType:
            return tr("Other");
            break;
        case QOpenGLDebugMessage::MarkerType:
            return tr("Marker");
            break;
        case QOpenGLDebugMessage::GroupPushType:
            return tr("Group push");
            break;
        case QOpenGLDebugMessage::GroupPopType:
            return tr("Group pop");
            break;
        default:
            return tr("Invalid");
            break;
        }
        return QVariant();
        break;
    case 4:
        return message.message();
        break;
    }
    return QVariant();
}

Qt::ItemFlags OpenGLDebugMessageModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant OpenGLDebugMessageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch(section) {
        case 0:
            return tr("Severity");
            break;
        case 1:
            return tr("ID");
            break;
        case 2:
            return tr("Source");
            break;
        case 3:
            return tr("Type");
            break;
        case 4:
            return tr("Message");
            break;
        }
    } else {
        return section + 1;
    }

    return QVariant();
}

QList<QOpenGLDebugMessage> OpenGLDebugMessageModel::messages() const
{
    return m_messages;
}

void OpenGLDebugMessageModel::clearMessages()
{
    beginResetModel();
    m_messages.clear();
    endResetModel();
}

void OpenGLDebugMessageModel::setMessages(const QList<QOpenGLDebugMessage> &messages)
{
    beginResetModel();
    m_messages = messages;
    endResetModel();
}

void OpenGLDebugMessageModel::appendMessage(const QOpenGLDebugMessage &message)
{
    beginInsertRows(QModelIndex(), rowCount(), 1);
    m_messages.append(message);
    endInsertRows();
}
