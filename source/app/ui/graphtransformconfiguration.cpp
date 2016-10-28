#include "graphtransformconfiguration.h"
#include "document.h"

#include "shared/utils/enumreflection.h"

REGISTER_QML_ENUM(GraphTransformType);
REGISTER_QML_ENUM(GraphTransformCreationState);

GraphTransformConfiguration::GraphTransformConfiguration()
{}

GraphTransformConfiguration::GraphTransformConfiguration(Document* document) :
    _document(document)
{}

GraphTransformConfiguration::GraphTransformConfiguration(Document* document,
                                                         bool enabled,
                                                         const QString& name,
                                                         const QString& fieldName,
                                                         const QString& op,
                                                         const QString& value) :
    _document(document),
    _enabled(enabled),
    _name(name),
    _fieldName(fieldName),
    _op(stringToEnum<ConditionFnOp>(op)),
    _value(value)
{}

GraphTransformConfiguration::GraphTransformConfiguration(const GraphTransformConfiguration& other) :
    QObject(),
    _document(other._document),
    _enabled(other._enabled),
    _name(other._name),
    _fieldName(other._fieldName),
    _op(other._op),
    _value(other._value),
    _locked(other._locked),
    _valid(other._valid),
    _creationState(other._creationState)
{}

GraphTransformConfiguration& GraphTransformConfiguration::operator=(const GraphTransformConfiguration& other)
{
    if(this != &other)
    {
        _document = other._document;
        _enabled = other._enabled;
        _name = other._name;
        _fieldName = other._fieldName;
        _op = other._op;
        _value = other._value;
        _locked = other._locked;
        _valid = other._valid;
        _creationState = other._creationState;
    }

    return *this;
}

void GraphTransformConfiguration::updateValidity()
{
    _valid = (_creationState >= GraphTransformCreationState::Enum::Created);
}

void GraphTransformConfiguration::updateCreationState()
{
    GraphTransformCreationState::Enum creationState;

    if(!_value.isEmpty())
        creationState = GraphTransformCreationState::Enum::Created;
    else if(_op != ConditionFnOp::None)
        creationState = GraphTransformCreationState::Enum::OperationSelected;
    else if(!_fieldName.isEmpty())
        creationState = GraphTransformCreationState::Enum::FieldSelected;
    else if(!_name.isEmpty())
        creationState = GraphTransformCreationState::Enum::TransformSelected;
    else
        creationState = GraphTransformCreationState::Enum::Uncreated;

    if(_creationState != creationState)
    {
        _creationState = creationState;
        updateValidity();
        emit creationStateChanged();
    }
}

bool GraphTransformConfiguration::enabled() const
{
    return _enabled;
}

QString GraphTransformConfiguration::name() const
{
    return _name;
}

QString GraphTransformConfiguration::fieldName() const
{
    return _fieldName;
}

const DataField& GraphTransformConfiguration::field() const
{
    return _document->dataFieldByName(_fieldName);
}

template<typename IntFn, typename FloatFn, typename T>
static T numericFieldFn(const DataField& field,
                        IntFn intFn,
                        FloatFn floatFn,
                        T defaultValue)
{
    switch(field.type())
    {
    case DataFieldType::IntNode:
    case DataFieldType::IntEdge:
    case DataFieldType::IntComponent:
        return intFn(field);

    case DataFieldType::FloatNode:
    case DataFieldType::FloatEdge:
    case DataFieldType::FloatComponent:
        return floatFn(field);

    default:
        return defaultValue;
    }
}

bool GraphTransformConfiguration::hasFieldRange() const
{
    return numericFieldFn(field(),
        [](const DataField& f) { return f.hasIntRange(); },
        [](const DataField& f) { return f.hasFloatRange(); },
        false);
}

double GraphTransformConfiguration::minFieldValue() const
{
    return numericFieldFn(field(),
        [](const DataField& f) { return f.intMin(); },
        [](const DataField& f) { return f.floatMin(); },
        0.0);
}

double GraphTransformConfiguration::maxFieldValue() const
{
    return numericFieldFn(field(),
        [](const DataField& f) { return f.intMax(); },
        [](const DataField& f) { return f.floatMax(); },
    0.0);
}

int GraphTransformConfiguration::type() const
{
    switch(field().type())
    {
    case DataFieldType::IntNode:
    case DataFieldType::IntEdge:
    case DataFieldType::IntComponent:
        return GraphTransformType::Enum::Int;

    case DataFieldType::FloatNode:
    case DataFieldType::FloatEdge:
    case DataFieldType::FloatComponent:
        return GraphTransformType::Enum::Float;

    case DataFieldType::StringNode:
    case DataFieldType::StringEdge:
    case DataFieldType::StringComponent:
        return GraphTransformType::Enum::String;

    default:
        return GraphTransformType::Enum::Unknown;
    }
}

QStringList GraphTransformConfiguration::availableTransformNames() const
{
    return _document->availableTransformNames();
}

QStringList GraphTransformConfiguration::availableDataFields() const
{
    return _document->availableDataFields(_name);
}

QStringList GraphTransformConfiguration::avaliableConditionFnOps() const
{
    return _document->avaliableConditionFnOps(_fieldName);
}

QString GraphTransformConfiguration::op() const
{
    return enumToString<ConditionFnOp>(_op);
}

ConditionFnOp GraphTransformConfiguration::opType() const
{
    return _op;
}

QString GraphTransformConfiguration::value() const
{
    return _value;
}

bool GraphTransformConfiguration::locked() const
{
    return _locked;
}

void GraphTransformConfiguration::setEnabled(bool enabled)
{
    if(_enabled != enabled)
    {
        _enabled = enabled;
        emit enabledChanged();
    }
}

void GraphTransformConfiguration::setName(const QString& name)
{
    if(_name != name)
    {
        _name = name;
        emit nameChanged();
        emit availableDataFieldsChanged();

        if(availableDataFields().size() == 1)
            setFieldName(availableDataFields().front());

        updateCreationState();
    }
}

void GraphTransformConfiguration::setFieldName(const QString& field)
{
    if(_fieldName != field)
    {
        _fieldName = field;
        emit fieldNameChanged();
        emit typeChanged();
        emit avaliableConditionFnOpsChanged();

        refreshField();
        updateCreationState();
    }
}

void GraphTransformConfiguration::setOp(ConditionFnOp op)
{
    if(_op != op)
    {
        _op = op;
        emit opChanged();

        updateCreationState();
    }
}

void GraphTransformConfiguration::setOp(const QString& op)
{
    setOp(stringToEnum<ConditionFnOp>(op));
}

void GraphTransformConfiguration::setValue(const QString& value)
{
    if(_value != value)
    {
        _value = value;
        emit valueChanged();

        updateCreationState();
    }
}

void GraphTransformConfiguration::setLocked(bool locked)
{
    if(_locked != locked)
    {
        _locked = locked;
        emit lockedChanged();
    }
}

void GraphTransformConfiguration::refreshField()
{
    emit hasFieldRangeChanged();
    emit minFieldValueChanged();
    emit maxFieldValueChanged();
}
