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
    height: 250
    minimumWidth: 800
    minimumHeight: 250

    property var document
    property string transformExpression

    ColumnLayout
    {
        id: layout

        anchors.fill: parent
        anchors.margins: Constants.margin

        GridLayout
        {
            columns: 5
            rows: 2

            Layout.fillWidth: true
            Layout.fillHeight: true

            ListBox
            {
                id: transformsList
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.rowSpan: 2

                onSelectedValueChanged:
                {
                    attributeList.model = document.availableAttributes(selectedValue);
                    updateTransformExpression();
                }
            }

            Label
            {
                Layout.topMargin: Constants.margin
                Layout.alignment: Qt.AlignTop
                Layout.rowSpan: 2
                text: qsTr("where")
            }

            ListBox
            {
                id: attributeList
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.rowSpan: 2

                onSelectedValueChanged:
                {
                    opList.updateModel(document.avaliableConditionFnOps(selectedValue));

                    if(attributeList.selectedValue !== undefined)
                    {
                        var parameterData = document.findTransformParameter(transformsList.selectedValue,
                                                                            attributeList.selectedValue);
                        parameterData.initialValue = "";
                        valueParameter.configure(parameterData);

                        attributeDescription.text = parameterData.description;
                    }
                    else
                    {
                        valueParameter.reset();
                        attributeDescription.text = "";
                    }

                    updateTransformExpression();
                }
            }

            ListBox
            {
                id: opList
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.rowSpan: 2
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

            Text
            {
                id: attributeDescription
                Layout.preferredWidth: valueParameter.width
                Layout.fillHeight: true

                verticalAlignment: Text.AlignVCenter
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
                elide: Text.ElideRight

                onLinkActivated: Qt.openUrlExternally(link);
            }
        }

        RowLayout
        {
            Label
            {
                id: transformExpressionDisplay
                Layout.fillWidth: true
            }

            Button
            {
                text: qsTr("OK")
                enabled: { return document.graphTransformIsValid(transformExpression); }
                onClicked: { root.accept(); }
            }

            Button
            {
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
        var displayExpression = "";

        if(transformsList.selectedValue !== undefined)
        {
            expression += "\"" + transformsList.selectedValue + "\"";
            displayExpression += transformsList.selectedValue;

            if(attributeList.selectedValue !== undefined)
            {
                expression += " where \"" + attributeList.selectedValue + "\"";
                displayExpression += " where " + attributeList.selectedValue;

                if(opList.selectedValue !== undefined)
                {
                    expression += " " + opList.selectedValue.value;
                    displayExpression += " " + TransformConfig.sanitiseOp(opList.selectedValue.value);

                    if(valueParameter.value.length > 0)
                    {
                        expression += " " + valueParameter.value;
                        displayExpression += " " + TransformConfig.roundTo3dp(valueParameter.value);
                    }
                }
            }
        }

        transformExpression = expression;
        transformExpressionDisplay.text = displayExpression;
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
    }
}
