import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import com.kajeka 1.0

import ".."
import "TransformConfig.js" as TransformConfig
import "../Constants.js" as Constants
import "../Utils.js" as Utils

import "../Controls"

Window
{
    id: root

    title: qsTr("Add Transform")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog
    width: 800
    height: 350
    minimumWidth: 800
    minimumHeight: 350

    property var document
    property string transformExpression

    property var _transform: undefined

    ColumnLayout
    {
        id: layout

        anchors.fill: parent
        anchors.margins: Constants.margin

        RowLayout
        {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListBox
            {
                id: transformsList
                Layout.fillWidth: true
                Layout.fillHeight: true

                onSelectedValueChanged:
                {
                    if(selectedValue !== undefined)
                    {
                        parametersRepeater.model = [];
                        parameters._values = {};
                        root._transform = document.transform(selectedValue);
                        attributeList.model = _transform.attributes;

                        if(_transform.parameters !== undefined)
                            parametersRepeater.model = Object.keys(_transform.parameters);
                    }

                    description.update();
                    updateTransformExpression();
                }
            }

            ColumnLayout
            {
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: 512
                spacing: 20

                Label
                {
                    visible: !parameters.visible && !condition.visible
                    Layout.fillWidth: visible
                    Layout.fillHeight: visible

                    horizontalAlignment: Qt.AlignCenter
                    verticalAlignment: Qt.AlignVCenter
                    font.pixelSize: 16
                    font.italic: true

                    text: _transform !== undefined && _transform.requirements === TransformRequirements.None ?
                              qsTr("No Parameters") : qsTr("Select A Transform")
                }

                ColumnLayout
                {
                    id: parameters
                    visible: _transform !== undefined &&
                             _transform.requirements & TransformRequirements.Parameters

                    Layout.fillWidth: visible
                    spacing: 20

                    property var _values

                    Repeater
                    {
                        id: parametersRepeater

                        delegate: Component
                        {
                            ColumnLayout
                            {
                                property var parameterData: _transform.parameters[modelData]

                                RowLayout
                                {
                                    id: parameterRowLayout
                                    Layout.fillWidth: true

                                    Label
                                    {
                                        Layout.alignment: Qt.AlignTop
                                        font.italic: true
                                        font.bold: true
                                        text: modelData
                                    }

                                    Item { Layout.fillWidth: true }
                                }

                                Text
                                {
                                    Layout.fillWidth: true
                                    text: parameterData.description
                                    textFormat: Text.RichText
                                    wrapMode: Text.WordWrap
                                    elide: Text.ElideRight
                                    onLinkActivated: Qt.openUrlExternally(link);
                                }

                                Component.onCompleted:
                                {
                                    var transformParameter = TransformConfig.createTransformParameter(document,
                                        parameterRowLayout, parameterData, updateTransformExpression);
                                    transformParameter.direction = Qt.Vertical;
                                    parameters._values[modelData] = transformParameter;
                                }
                            }
                        }
                    }

                    function valueOf(parameterName)
                    {
                        if(_values === undefined)
                            return "\"\"";

                        var transformParameter = _values[parameterName];

                        if(transformParameter === undefined)
                            return "\"\"";

                        return transformParameter.value;
                    }
                }

                RowLayout
                {
                    id: condition
                    visible: _transform !== undefined &&
                             _transform.requirements & TransformRequirements.Condition

                    Layout.fillWidth: visible
                    Layout.fillHeight: visible
                    Layout.minimumHeight: 128

                    Label
                    {
                        Layout.topMargin: Constants.margin
                        Layout.alignment: Qt.AlignTop
                        text: qsTr("where")
                    }

                    ListBox
                    {
                        id: attributeList
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        onSelectedValueChanged:
                        {
                            if(selectedValue !== undefined)
                            {
                                opList.updateModel(document.attribute(selectedValue).ops);

                                var parameterData = document.findTransformParameter(transformsList.selectedValue,
                                                                                    attributeList.selectedValue);
                                parameterData.initialValue = "";
                                valueParameter.configure(parameterData);
                            }
                            else
                                valueParameter.reset();

                            description.update();
                            updateTransformExpression();
                        }
                    }

                    ListBox
                    {
                        id: opList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredWidth: 120

                        onSelectedValueChanged: { updateTransformExpression(); }

                        Component { id: modelComponent; ListModel {} }

                        function updateModel(ops)
                        {
                            var newModel = modelComponent.createObject();

                            for(var i = 0; i < ops.length; i++)
                            {
                                var item = { display: TransformConfig.sanitiseOp(ops[i]), value: ops[i] };
                                newModel.append(item);
                            }

                            opList.model = newModel;
                        }
                    }

                    TransformParameter
                    {
                        id: valueParameter
                        Layout.topMargin: Constants.margin
                        Layout.alignment: Qt.AlignTop

                        updateValueImmediately: true
                        direction: Qt.Vertical
                        onValueChanged: { updateTransformExpression(); }
                    }
                }
            }
        }

        RowLayout
        {
            Layout.fillHeight: true
            Layout.preferredHeight: 64

            Text
            {
                id: description
                Layout.fillWidth: true

                textFormat: Text.RichText
                wrapMode: Text.WordWrap
                elide: Text.ElideRight

                onLinkActivated: Qt.openUrlExternally(link);

                function update()
                {
                    text = "";

                    if(_transform !== undefined)
                    {
                        text += _transform.description;

                        if(attributeList.selectedValue !== undefined)
                        {
                            var parameterData = document.findTransformParameter(transformsList.selectedValue,
                                                                                attributeList.selectedValue);

                            text += "<br><br>" + parameterData.description;
                        }
                    }
                }
            }

            Button
            {
                Layout.alignment: Qt.AlignBottom
                text: qsTr("OK")
                enabled: { return document.graphTransformIsValid(transformExpression); }
                onClicked: { root.accept(); }
            }

            Button
            {
                Layout.alignment: Qt.AlignBottom
                text: qsTr("Cancel")
                onClicked: { root.reject(); }
            }
        }

        Keys.onPressed:
        {
            event.accepted = true;
            switch(event.key)
            {
            case Qt.Key_Escape:
            case Qt.Key_Back:
                reject();
                break;

            case Qt.Key_Enter:
            case Qt.Key_Return:
                accept();
                break;

            default:
                event.accepted = false;
            }
        }
    }

    function accept()
    {
        accepted();
        root.close();
    }

    function reject()
    {
        rejected();
        root.close();
    }

    signal accepted()
    signal rejected()

    function updateTransformExpression()
    {
        var expression = "";

        if(transformsList.selectedValue !== undefined)
        {
            expression += "\"" + transformsList.selectedValue + "\"";

            if(Object.keys(_transform.parameters).length > 0)
            {
                expression += " with";

                for(var parameterName in _transform.parameters)
                    expression += " " + parameterName + " = " + parameters.valueOf(parameterName);
            }

            if(attributeList.selectedValue !== undefined)
            {
                expression += " where \"" + attributeList.selectedValue + "\"";

                if(opList.selectedValue !== undefined)
                {
                    expression += " " + opList.selectedValue.value;

                    if(valueParameter.value.length > 0)
                        expression += " " + valueParameter.value;
                }
            }
        }

        transformExpression = expression;
    }

    onAccepted:
    {
        updateTransformExpression();
        document.appendGraphTransform(transformExpression);
        document.updateGraphTransforms();
    }

    onVisibleChanged:
    {
        transformExpression.text = "";
        transformsList.model = document.availableTransformNames();
        parametersRepeater.model = undefined;
        _transform = undefined;
    }
}
