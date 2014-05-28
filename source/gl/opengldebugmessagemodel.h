#ifndef OPENGLDEBUGMESSAGEMODEL_H
#define OPENGLDEBUGMESSAGEMODEL_H

#include <QAbstractTableModel>
#include <QOpenGLDebugMessage>

class OpenGLDebugMessageModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit OpenGLDebugMessageModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QList<QOpenGLDebugMessage> messages() const;

public slots:
    void clearMessages();
    void setMessages(const QList<QOpenGLDebugMessage> &messages);
    void appendMessage(const QOpenGLDebugMessage &message);

private:
    QList<QOpenGLDebugMessage> m_messages;
};

#endif // OPENGLDEBUGMESSAGEMODEL_H
