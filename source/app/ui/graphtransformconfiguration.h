#ifndef GRAPHTRANSFORMCONFIGURATION_H
#define GRAPHTRANSFORMCONFIGURATION_H

#include "../transform/datafield.h"

#include "../application.h"
#include "../utils/qmlenum.h"

#include <QObject>
#include <QString>

class Document;

DEFINE_QML_ENUM(GraphTransformType,
                Unknown, Int, Float, String);

DEFINE_QML_ENUM(GraphTransformCreationState,
                Uncreated,
                TransformSelected,
                FieldSelected,
                OperationSelected,
                Created);

class GraphTransformConfiguration : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Document* document READ document CONSTANT)
    Q_PROPERTY(bool transformEnabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString fieldName READ fieldName WRITE setFieldName NOTIFY fieldNameChanged)
    Q_PROPERTY(QString op READ op WRITE setOp NOTIFY opChanged)
    Q_PROPERTY(QString fieldValue READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(bool locked READ locked WRITE setLocked NOTIFY lockedChanged)

    Q_PROPERTY(bool hasFieldRange READ hasFieldRange NOTIFY hasFieldRangeChanged)
    Q_PROPERTY(double minFieldValue READ minFieldValue NOTIFY minFieldValueChanged)
    Q_PROPERTY(double maxFieldValue READ maxFieldValue NOTIFY maxFieldValueChanged)

    Q_PROPERTY(int type READ type NOTIFY typeChanged)
    Q_PROPERTY(QStringList availableTransformNames READ availableTransformNames NOTIFY availableTransformNamesChanged)
    Q_PROPERTY(QStringList availableDataFields READ availableDataFields NOTIFY availableDataFieldsChanged)
    Q_PROPERTY(QStringList avaliableConditionFnOps READ avaliableConditionFnOps NOTIFY avaliableConditionFnOpsChanged)

    Q_PROPERTY(GraphTransformCreationState::Enum creationState READ creationState NOTIFY creationStateChanged)

private:
    Document* _document = nullptr;

    bool _enabled = true;
    QString _name;
    QString _fieldName;
    ConditionFnOp _op = ConditionFnOp::None;
    QString _value;
    bool _locked = false;

    bool _valid = false;
    GraphTransformCreationState::Enum _creationState = GraphTransformCreationState::Enum::Uncreated;

    void updateValidity();
    void updateCreationState();

public:
    GraphTransformConfiguration();

    explicit GraphTransformConfiguration(Document* document);

    GraphTransformConfiguration(Document* document,
                                bool enabled,
                                const QString& name,
                                const QString& fieldName,
                                const QString& op,
                                const QString& value);

    GraphTransformConfiguration(const GraphTransformConfiguration& other);
    GraphTransformConfiguration& operator=(const GraphTransformConfiguration& other);

    Document* document() const { return _document; }
    bool enabled() const;
    QString name() const;
    QString fieldName() const;
    QString op() const;
    ConditionFnOp opType() const;
    QString value() const;
    bool locked() const;

    void setEnabled(bool enabled);
    void setName(const QString& name);
    void setFieldName(const QString& fieldName);
    void setOp(ConditionFnOp op);
    void setOp(const QString& op);
    void setValue(const QString& value);
    void setLocked(bool locked);

    void refreshField();

    const DataField& field() const;

    bool hasFieldRange() const;
    bool hasMinFieldValue() const;
    bool hasMaxFieldValue() const;
    double minFieldValue() const;
    double maxFieldValue() const;

    int type() const;
    QStringList availableTransformNames() const;
    QStringList availableDataFields() const;
    QStringList avaliableConditionFnOps() const;

    bool valid() const { return _valid; }
    GraphTransformCreationState::Enum creationState() const { return _creationState; }

signals:
    void enabledChanged();
    void nameChanged();
    void fieldNameChanged();
    void opChanged();
    void valueChanged();
    void lockedChanged();

    void hasFieldRangeChanged();
    void minFieldValueChanged();
    void maxFieldValueChanged();

    void typeChanged();
    void availableTransformNamesChanged();
    void availableDataFieldsChanged();
    void avaliableConditionFnOpsChanged();

    void creationStateChanged();
};

#endif // GRAPHTRANSFORMCONFIGURATION_H

