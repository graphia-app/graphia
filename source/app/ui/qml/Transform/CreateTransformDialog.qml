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
    property var defaultVisualisations

    property var _transform: undefined
    property int _numParameters: _transform !== undefined ? Object.keys(_transform.parameters).length : 0
    property int _numDeclaredAttributes: _transform !== undefined ?
                                             Object.keys(_transform.declaredAttributes).length : 0

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
                Layout.preferredWidth: 192
                Layout.fillHeight: true

                onSelectedValueChanged:
                {
                    if(selectedValue !== undefined)
                    {
                        parametersRepeater.model = [];
                        parameters._values = {};

                        declaredAttributesRepeater.model = [];
                        declaredAttributes._visualisations = {};

                        root._transform = document.transform(selectedValue);
                        lhsAttributeList.model = document.availableAttributes(root._transform.elementType);
                        valueRadioButton.checked = true;
                        rhsAttributeList.model = undefined;

                        if(_transform.parameters !== undefined)
                            parametersRepeater.model = Object.keys(_transform.parameters);

                        if(_transform.declaredAttributes !== undefined)
                            declaredAttributesRepeater.model = Object.keys(_transform.declaredAttributes);
                    }

                    description.update();
                    updateTransformExpression();
                }
            }

            Label
            {
                visible: !scrollView.visible
                Layout.fillWidth: visible
                Layout.fillHeight: visible

                horizontalAlignment: Qt.AlignCenter
                verticalAlignment: Qt.AlignVCenter
                font.pixelSize: 16
                font.italic: true

                text: _transform !== undefined && _numParameters === 0 ?
                          qsTr("No Parameters Required") : qsTr("Select A Transform")
            }

            ScrollView
            {
                id: scrollView

                property bool _scrollBarVisible: (parameters.enabled || declaredAttributes.enabled) && condition.enabled
                property int margin: _scrollBarVisible ? 8 : 0
                frameVisible: _scrollBarVisible

                visible: parameters.enabled || declaredAttributes.enabled || condition.enabled

                Layout.fillWidth: visible
                Layout.fillHeight: visible

                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                verticalScrollBarPolicy: Qt.ScrollBarAsNeeded

                ColumnLayout
                {
                    width: scrollView.viewport.width
                    spacing: 20

                    RowLayout
                    {
                        id: condition
                        enabled: _transform !== undefined && _transform.requiresCondition
                        visible: enabled

                        Layout.margins: scrollView.margin
                        Layout.fillWidth: visible
                        Layout.minimumHeight:
                        {
                            var conditionHeight = scrollView.viewport.height - (scrollView.margin * 2);

                            if(parameters.enabled || declaredAttributes.enabled)
                                conditionHeight *= 0.5;

                            return Math.max(conditionHeight, 128);
                        }

                        Label
                        {
                            Layout.topMargin: Constants.margin
                            Layout.alignment: Qt.AlignTop
                            text: qsTr("where")
                        }

                        TreeBox
                        {
                            id: lhsAttributeList
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            onSelectedValueChanged:
                            {
                                if(selectedValue !== undefined)
                                {
                                    opList.updateModel(document.attribute(selectedValue).ops);

                                    var parameterData = document.findTransformParameter(transformsList.selectedValue,
                                                                                        lhsAttributeList.selectedValue);
                                    rhs.configure(parameterData);
                                }
                                else
                                {
                                    opList.updateModel();
                                    valueParameter.reset();
                                }

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
                                if(ops === undefined)
                                {
                                    opList.model = undefined;
                                    return;
                                }

                                var newModel = modelComponent.createObject();

                                for(var i = 0; i < ops.length; i++)
                                {
                                    var item =
                                    {
                                        display: TransformConfig.sanitiseOp(ops[i]),
                                        value: ops[i],
                                        unary: document.opIsUnary(ops[i])
                                    };

                                    newModel.append(item);
                                }

                                opList.model = newModel;
                            }
                        }

                        GridLayout
                        {
                            id: rhs

                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            enabled: opList.selectedValue !== undefined &&
                                     !opList.selectedValue.unary

                            columns: 2
                            ExclusiveGroup { id: rhsGroup }

                            RadioButton
                            {
                                id: valueRadioButton
                                Layout.alignment: Qt.AlignTop

                                checked: true
                                exclusiveGroup: rhsGroup
                            }

                            TransformParameter
                            {
                                id: valueParameter
                                Layout.fillWidth: true

                                enabled: valueRadioButton.checked
                                updateValueImmediately: true
                                direction: Qt.Vertical
                                fillWidth: true
                                onValueChanged: { updateTransformExpression(); }
                            }

                            RadioButton
                            {
                                id: attributeRadioButton
                                Layout.alignment: Qt.AlignTop

                                exclusiveGroup: rhsGroup
                            }

                            TreeBox
                            {
                                id: rhsAttributeList
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                enabled: attributeRadioButton.checked
                                onSelectedValueChanged: { updateTransformExpression(); }
                            }

                            function configure(parameterData)
                            {
                                valueParameter.configure(parameterData);

                                if(parameterData.valueType !== undefined)
                                {
                                    rhsAttributeList.model = document.availableAttributes(
                                                root._transform.elementType, parameterData.valueType);
                                }
                                else
                                    rhsAttributeList.model = undefined;
                            }

                            function value()
                            {
                                if(valueRadioButton.checked)
                                    return valueParameter.value;
                                else if(attributeRadioButton.checked)
                                    return "$\"" + rhsAttributeList.selectedValue + "\"";

                                return "";
                            }
                        }
                    }

                    ColumnLayout
                    {
                        id: parameters
                        enabled: _transform !== undefined && _numParameters > 0
                        visible: enabled

                        Layout.margins: scrollView.margin
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

                    ColumnLayout
                    {
                        id: declaredAttributes
                        enabled: _transform !== undefined && _numDeclaredAttributes > 0
                        visible: enabled

                        Layout.margins: scrollView.margin
                        Layout.fillWidth: visible
                        spacing: 20

                        property var _visualisations

                        Label
                        {
                            Layout.alignment: Qt.AlignTop
                            font.italic: true
                            font.bold: true
                            text: qsTr("Visualisations")
                        }

                        Repeater
                        {
                            id: declaredAttributesRepeater

                            delegate: Component
                            {
                                ColumnLayout
                                {
                                    property var declaredAttribute: _transform !== undefined ?
                                                                        _transform.declaredAttributes[modelData] : undefined

                                    RowLayout
                                    {
                                        Layout.fillWidth: true

                                        Label
                                        {
                                            text: modelData
                                        }

                                        ComboBox
                                        {
                                            id: visualisationsComboBox
                                            editable: false
                                            onCurrentIndexChanged:
                                            {
                                                var value = currentText;
                                                if(value === "None")
                                                    value = "";

                                                declaredAttributes._visualisations[modelData] = value;
                                            }
                                        }

                                        Item { Layout.fillWidth: true }
                                    }

                                    Component.onCompleted:
                                    {
                                        var visualisationChannelNames = document.availableVisualisationChannelNames(declaredAttribute.valueType);
                                        visualisationsComboBox.model = ["None"].concat(visualisationChannelNames);
                                        visualisationsComboBox.currentIndex = visualisationsComboBox.model.indexOf(declaredAttribute.defaultVisualisation);
                                        declaredAttributes._visualisations[modelData] = declaredAttribute.defaultVisualisation;
                                    }
                                }
                            }
                        }

                        function selectedVisualisation(attributeName)
                        {
                            if(_visualisations === undefined)
                                return "";

                            var visualisationChannelName = _visualisations[attributeName];

                            if(visualisationChannelName === undefined)
                                return "";

                            return visualisationChannelName;
                        }
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

                        if(lhsAttributeList.selectedValue !== undefined)
                        {
                            var parameterData = document.findTransformParameter(transformsList.selectedValue,
                                                                                lhsAttributeList.selectedValue);

                            if(parameterData.description !== undefined)
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

            if(parameters !== undefined && Object.keys(_transform.parameters).length > 0)
            {
                expression += " with";

                for(var parameterName in _transform.parameters)
                    expression += " " + parameterName + " = " + parameters.valueOf(parameterName);
            }

            if(lhsAttributeList.selectedValue !== undefined)
            {
                expression += " where $\"" + lhsAttributeList.selectedValue + "\"";

                if(opList.selectedValue !== undefined)
                {
                    expression += " " + opList.selectedValue.value;
                    var rhsValue = rhs.value();

                    if(!opList.selectedValue.unary && rhsValue.length > 0)
                        expression += " " + rhsValue;
                }
            }
        }

        transformExpression = expression;
    }

    function updateDefaultVisualisations()
    {
        defaultVisualisations = [];

        Object.keys(_transform.declaredAttributes).forEach(function(attributeName)
        {
            var channelName = declaredAttributes.selectedVisualisation(attributeName);

            if(channelName.length > 0)
            {
                var expression = "\"" + attributeName + "\" \"" + channelName + "\"";

                var valueType = _transform.declaredAttributes[attributeName].valueType;
                var parameters = document.visualisationDefaultParameters(valueType,
                                                                         channelName);

                if(Object.keys(parameters).length !== 0)
                    expression += " with ";

                for(var key in parameters)
                    expression += " " + key + " = " + parameters[key];

                defaultVisualisations.push(expression);
            }
        });
    }

    onAccepted:
    {
        updateTransformExpression();
        document.appendGraphTransform(transformExpression);

        updateDefaultVisualisations();
        for(var i = 0; i < defaultVisualisations.length; i++)
            document.appendVisualisation(defaultVisualisations[i]);

        document.update();
    }

    onVisibleChanged:
    {
        transformExpression.text = "";
        defaultVisualisations = [];
        transformsList.model = document.availableTransformNames();
        lhsAttributeList.model = rhsAttributeList.model = undefined;
        opList.model = undefined;
        parametersRepeater.model = undefined;
        _transform = undefined;
    }
}
